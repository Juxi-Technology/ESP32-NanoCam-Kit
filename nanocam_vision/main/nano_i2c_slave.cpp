#ifdef CONFIG_NANOCAM_ENABLE_I2C_SLAVE

#include "nano_i2c_slave.h"
#include "nano_config.h"
#include <string.h>
#include <stdio.h>
#include "esp_log.h"
#include "esp_err.h"
#include "driver/i2c_slave.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "nano_i2c";

#define I2C_SLAVE_PORT   I2C_NUM_0
#define I2C_TASK_PRIO    5
#define I2C_TASK_STACK   2048

static i2c_slave_bus_handle_t s_i2c_bus = NULL;

// Register map
enum {
    REG_HFLIP       = 0x01,
    REG_VFLIP       = 0x02,
    REG_MODE_SELECT = 0x03,
    REG_MODE_RESET  = 0x04,
    REG_VIRTUAL_KEY = 0x08,

    REG_LX_H = 0x20, REG_LX_L = 0x21,
    REG_LY_H = 0x22, REG_LY_L = 0x23,
    REG_RX_H = 0x24, REG_RX_L = 0x25,
    REG_RY_H = 0x26, REG_RY_L = 0x27,
    REG_MX_H = 0x28, REG_MX_L = 0x29,
    REG_MY_H = 0x2A, REG_MY_L = 0x2B,
    REG_ID_H = 0x2C, REG_ID_L = 0x2D,
    REG_AREA_H = 0x2E, REG_AREA_L = 0x2F,
};

// Detection data
static struct {
    int16_t lx, ly, rx, ry;
    int16_t mx, my;
    int16_t id;
    int16_t area;
} s_detect = {0};

static uint8_t s_reg_addr = 0x28;  // default read: center X

// ---- Register read helper ----
static uint8_t read_reg(uint8_t reg) {
    switch (reg) {
    case REG_LX_H: return (uint8_t)(s_detect.lx >> 8);
    case REG_LX_L: return (uint8_t)(s_detect.lx & 0xFF);
    case REG_LY_H: return (uint8_t)(s_detect.ly >> 8);
    case REG_LY_L: return (uint8_t)(s_detect.ly & 0xFF);
    case REG_RX_H: return (uint8_t)(s_detect.rx >> 8);
    case REG_RX_L: return (uint8_t)(s_detect.rx & 0xFF);
    case REG_RY_H: return (uint8_t)(s_detect.ry >> 8);
    case REG_RY_L: return (uint8_t)(s_detect.ry & 0xFF);
    case REG_MX_H: return (uint8_t)(s_detect.mx >> 8);
    case REG_MX_L: return (uint8_t)(s_detect.mx & 0xFF);
    case REG_MY_H: return (uint8_t)(s_detect.my >> 8);
    case REG_MY_L: return (uint8_t)(s_detect.my & 0xFF);
    case REG_ID_H: return (uint8_t)(s_detect.id >> 8);
    case REG_ID_L: return (uint8_t)(s_detect.id & 0xFF);
    case REG_AREA_H: return (uint8_t)(s_detect.area >> 8);
    case REG_AREA_L: return (uint8_t)(s_detect.area & 0xFF);
    default: return 0x00;
    }
}

// ---- Pre-fill TX buffer with current register data ----
// Called after detection update so master reads always get fresh data,
// even if the I2C receive task hasn't caught up with register writes.
static void prefill_tx_buffer(void) {
    if (!s_i2c_bus) return;
    uint8_t data[4];
    data[0] = read_reg(s_reg_addr);
    data[1] = read_reg(s_reg_addr + 1);
    data[2] = read_reg(s_reg_addr + 2);
    data[3] = read_reg(s_reg_addr + 3);
    i2c_slave_transmit(s_i2c_bus, data, sizeof(data), 0);
}

// ---- I2C slave polling task ----
// Waits for master write (register address), then updates TX buffer.
// Falls back gracefully: if task can't keep up, prefill_tx_buffer() ensures
// the last known detection data is always served.
static void i2c_slave_task(void *arg) {
    uint8_t rx_buf[8];

    // Initial pre-fill so master can read immediately
    prefill_tx_buffer();

    while (1) {
        // Block until master writes to us (register address)
        esp_err_t err = i2c_slave_receive(s_i2c_bus, rx_buf, sizeof(rx_buf),
                                           pdMS_TO_TICKS(500));
        if (err == ESP_OK) {
            // Store last byte of write as register address
            // (Arduino Wire library writes register addr as 1-byte payload)
            bool addr_changed = (rx_buf[0] != s_reg_addr);
            s_reg_addr = rx_buf[0];

            // Immediately re-fill buffer with register-specific data
            if (addr_changed) {
                prefill_tx_buffer();
            }
        }
        // Short yield to avoid starving other tasks in error state
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

// ---- Public API ----

esp_err_t nano_i2c_slave_init(void)
{
    i2c_slave_config_t cfg = {
        .i2c_port       = I2C_SLAVE_PORT,
        .sda_io_num     = NANOCAM_I2C_SDA,
        .scl_io_num     = NANOCAM_I2C_SCL,
        .sda_pullup_en  = true,
        .scl_pullup_en  = true,
        .addr_bit_len   = I2C_ADDR_BIT_LEN_7,
        .slave_addr     = NANO_I2C_SLAVE_ADDR,
    };

    esp_err_t err = i2c_new_slave_bus(I2C_SLAVE_PORT, &cfg, &s_i2c_bus);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "i2c_new_slave_bus failed: 0x%x", err);
        return err;
    }

    BaseType_t ret = xTaskCreate(i2c_slave_task, "i2c_slv",
                                  I2C_TASK_STACK, NULL, I2C_TASK_PRIO, NULL);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create I2C slave task");
        return ESP_ERR_NO_MEM;
    }

    ESP_LOGI(TAG, "I2C slave ready at 0x%02X (SDA=%d, SCL=%d)",
             NANO_I2C_SLAVE_ADDR, NANOCAM_I2C_SDA, NANOCAM_I2C_SCL);
    return ESP_OK;
}

void nano_i2c_slave_set_detection(int16_t x, int16_t y, int16_t w, int16_t h)
{
    s_detect.lx = x;
    s_detect.ly = y;
    s_detect.rx = x + w;
    s_detect.ry = y + h;
    s_detect.mx = x + w / 2;
    s_detect.my = y + h / 2;
    s_detect.area = w * h;

    // Pre-load TX buffer so the next master read gets fresh data
    prefill_tx_buffer();
}

void nano_i2c_slave_set_id(int16_t id)
{
    s_detect.id = id;
    prefill_tx_buffer();
}

bool nano_i2c_slave_get_detection(int16_t *x, int16_t *y, int16_t *w, int16_t *h, int16_t *id)
{
    if (x)  *x  = s_detect.lx;
    if (y)  *y  = s_detect.ly;
    if (w)  *w  = s_detect.rx - s_detect.lx;
    if (h)  *h  = s_detect.ry - s_detect.ly;
    if (id) *id = s_detect.id;
    return s_detect.area > 0;
}

#else

#include "nano_i2c_slave.h"
#include "esp_log.h"
esp_err_t nano_i2c_slave_init(void) {
    ESP_LOGI("nano_i2c", "I2C slave disabled");
    return ESP_OK;
}
void nano_i2c_slave_set_detection(int16_t x, int16_t y, int16_t w, int16_t h) {}
void nano_i2c_slave_set_id(int16_t id) {}
bool nano_i2c_slave_get_detection(int16_t *x, int16_t *y, int16_t *w, int16_t *h, int16_t *id) { return false; }

#endif