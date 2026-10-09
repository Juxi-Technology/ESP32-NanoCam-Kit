#include "nano_serial.h"
#include "nano_config.h"
#include "nano_camera.h"
#include "sdkconfig.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include "esp_log.h"
#include "driver/uart.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "nano_human_face_recognition.hpp"
static const char *(*s_sta_ip_cb)(void) = NULL;
static const char *VERSION = "0.2.0";

#define UART_NUM      UART_NUM_0
static const char *NVS_NS = "nanocam";

static SemaphoreHandle_t s_serial_mutex = NULL;

static void ensure_mutex(void) {
    if (s_serial_mutex == NULL) {
        s_serial_mutex = xSemaphoreCreateMutex();
    }
}

static char  s_sta_ssid[32]  = "";
static char  s_sta_pass[64]  = "";
static char  s_ap_ssid[32]   = "NanoCam-AP";
static char  s_ap_pass[64]   = "12345678";
static int   s_wifi_mode     = 0;
static int   s_ai_mode       = CONFIG_NANOCAM_DEFAULT_AI_MODE;
static char  s_server_url[128] = "";
static char  s_api_key[128]    = "";
static char  s_ros_ip[32]      = "";

static void load_settings(void) {
    nvs_handle_t h;
    if (nvs_open(NVS_NS, NVS_READONLY, &h) != ESP_OK) return;
    size_t len;
    len = sizeof(s_sta_ssid); nvs_get_str(h, "stassid", s_sta_ssid, &len);
    len = sizeof(s_sta_pass); nvs_get_str(h, "stapass", s_sta_pass, &len);
    len = sizeof(s_ap_ssid);  nvs_get_str(h, "apssid",  s_ap_ssid,  &len);
    len = sizeof(s_ap_pass);  nvs_get_str(h, "appass",  s_ap_pass,  &len);
    uint8_t v;
    if (nvs_get_u8(h, "wmode",  &v) == ESP_OK) s_wifi_mode = v;
    if (nvs_get_u8(h, "aimode", &v) == ESP_OK) s_ai_mode   = v;
    len = sizeof(s_server_url); nvs_get_str(h, "server", s_server_url, &len);
    len = sizeof(s_api_key);  nvs_get_str(h, "apikey", s_api_key, &len);
    len = sizeof(s_ros_ip);   nvs_get_str(h, "rosip", s_ros_ip, &len);
    nvs_close(h);
}

static void save_u8(const char *key, uint8_t val) {
    nvs_handle_t h;
    esp_err_t err = nvs_open(NVS_NS, NVS_READWRITE, &h);
    if (err != ESP_OK) return;
    err = nvs_set_u8(h, key, val);
    err = nvs_commit(h);
    nvs_close(h);
}
static void save_str(const char *key, const char *val) {
    nvs_handle_t h;
    esp_err_t err = nvs_open(NVS_NS, NVS_READWRITE, &h);
    if (err != ESP_OK) return;
    err = nvs_set_str(h, key, val);
    err = nvs_commit(h);
    nvs_close(h);
}
static void nvs_erase_all_settings(void) {
    nvs_handle_t h;
    if (nvs_open(NVS_NS, NVS_READWRITE, &h) != ESP_OK) return;
    nvs_erase_all(h); nvs_commit(h); nvs_close(h);
}

int nano_serial_get_ai_mode(void)   { ensure_mutex(); xSemaphoreTake(s_serial_mutex, portMAX_DELAY); int v = s_ai_mode; xSemaphoreGive(s_serial_mutex); return v; }
int nano_serial_get_wifi_mode(void) { ensure_mutex(); xSemaphoreTake(s_serial_mutex, portMAX_DELAY); int v = s_wifi_mode; xSemaphoreGive(s_serial_mutex); return v; }
const char *nano_serial_get_sta_ssid(void) {
    static char buf[33]; ensure_mutex();
    xSemaphoreTake(s_serial_mutex, portMAX_DELAY);
    strncpy(buf, s_sta_ssid, 32); buf[32] = '\0';
    xSemaphoreGive(s_serial_mutex);
    return buf;
}
const char *nano_serial_get_sta_pass(void) {
    static char buf[65]; ensure_mutex();
    xSemaphoreTake(s_serial_mutex, portMAX_DELAY);
    strncpy(buf, s_sta_pass, 64); buf[64] = '\0';
    xSemaphoreGive(s_serial_mutex);
    return buf;
}
const char *nano_serial_get_ap_ssid(void) {
    static char buf[33]; ensure_mutex();
    xSemaphoreTake(s_serial_mutex, portMAX_DELAY);
    strncpy(buf, s_ap_ssid, 32); buf[32] = '\0';
    xSemaphoreGive(s_serial_mutex);
    return buf;
}
const char *nano_serial_get_ap_pass(void) {
    static char buf[65]; ensure_mutex();
    xSemaphoreTake(s_serial_mutex, portMAX_DELAY);
    strncpy(buf, s_ap_pass, 64); buf[64] = '\0';
    xSemaphoreGive(s_serial_mutex);
    return buf;
}
const char *nano_serial_get_server(void) {
    static char buf[129]; ensure_mutex();
    xSemaphoreTake(s_serial_mutex, portMAX_DELAY);
    strncpy(buf, s_server_url, 128); buf[128] = '\0';
    xSemaphoreGive(s_serial_mutex);
    return buf;
}
const char *nano_serial_get_api_key(void) {
    static char buf[129]; ensure_mutex();
    xSemaphoreTake(s_serial_mutex, portMAX_DELAY);
    strncpy(buf, s_api_key, 128); buf[128] = '\0';
    xSemaphoreGive(s_serial_mutex);
    return buf;
}

const char *nano_serial_get_ros_ip(void) {
    static char buf[33]; ensure_mutex();
    xSemaphoreTake(s_serial_mutex, portMAX_DELAY);
    strncpy(buf, s_ros_ip, 31); buf[31] = '\0';
    xSemaphoreGive(s_serial_mutex);
    return buf;
}

static bool cmd_is(const char *cmd, const char *prefix) {
    while (*prefix) {
        if (*cmd == '\0') return false;
        char c = *cmd++;
        char p = *prefix++;
        if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
        if (p >= 'A' && p <= 'Z') p += 'a' - 'A';
        if (c != p) return false;
    }
    return true;
}
static bool cmd_eq(const char *cmd, const char *s) {
    return cmd_is(cmd, s) && cmd[strlen(s)] == '\0';
}

static void handle_command(const char *cmd) {
    char resp[256] = {0};

    if (cmd_is(cmd, "sta_ssid:")) {
        const char *val = cmd + 9;
        if (strlen(val) > 0 && strlen(val) <= 31) {
            ensure_mutex();
            xSemaphoreTake(s_serial_mutex, portMAX_DELAY);
            strncpy(s_sta_ssid, val, 31);
            save_str("stassid", s_sta_ssid);
            xSemaphoreGive(s_serial_mutex);
            snprintf(resp, sizeof(resp), "OK\r\n");
        } else {
            snprintf(resp, sizeof(resp), "fail,ssid is null!\r\n");
        }
    }
    else if (cmd_is(cmd, "sta_pd:")) {
        ensure_mutex();
        xSemaphoreTake(s_serial_mutex, portMAX_DELAY);
        strncpy(s_sta_pass, cmd + 7, 63);
        save_str("stapass", s_sta_pass);
        xSemaphoreGive(s_serial_mutex);
        snprintf(resp, sizeof(resp), "OK (saved, use nano_reboot to apply)\r\n");
    }
    else if (cmd_is(cmd, "ap_ssid:")) {
        const char *val = cmd + 8;
        if (strlen(val) > 0 && strlen(val) <= 31) {
            ensure_mutex();
            xSemaphoreTake(s_serial_mutex, portMAX_DELAY);
            strncpy(s_ap_ssid, val, 31);
            save_str("apssid", s_ap_ssid);
            xSemaphoreGive(s_serial_mutex);
            snprintf(resp, sizeof(resp), "OK\r\n");
        } else {
            snprintf(resp, sizeof(resp), "fail,AP_Name is null!\r\n");
        }
    }
    else if (cmd_is(cmd, "ap_pd:")) {
        ensure_mutex();
        xSemaphoreTake(s_serial_mutex, portMAX_DELAY);
        strncpy(s_ap_pass, cmd + 6, 63);
        save_str("appass", s_ap_pass);
        xSemaphoreGive(s_serial_mutex);
        snprintf(resp, sizeof(resp), "OK (saved, use nano_reboot to apply)\r\n");
    }
    else if (cmd_is(cmd, "wifi_mode:")) {
        int mode = cmd[10] - '0';
        if (mode < 0) mode = 0;
        if (mode > 2) mode = 2;
        ensure_mutex();
        xSemaphoreTake(s_serial_mutex, portMAX_DELAY);
        s_wifi_mode = mode;
        save_u8("wmode", mode);
        xSemaphoreGive(s_serial_mutex);
        snprintf(resp, sizeof(resp), "OK wifi_mode:%d (saved, use nano_reboot to apply)\r\n", mode);
    }
    else if (cmd_is(cmd, "ai_mode:")) {
        int mode = cmd[8] - '0';
        if (mode < 0) mode = 0;
        if (mode > 7) mode = 0;
        ensure_mutex();
        xSemaphoreTake(s_serial_mutex, portMAX_DELAY);
        s_ai_mode = mode;
        save_u8("aimode", mode);
        xSemaphoreGive(s_serial_mutex);
        snprintf(resp, sizeof(resp), "\r\n>>> OK ai_mode:%d (saved, use nano_reboot to apply)\r\n", mode);
    }
    else if (cmd_eq(cmd, "sta_ip")) {
        if (s_sta_ip_cb) {
            const char *ip = s_sta_ip_cb();
            snprintf(resp, sizeof(resp), "sta_ip:%s\r\n", ip[0] ? ip : "0.0.0.0");
        } else {
            snprintf(resp, sizeof(resp), "sta_ip:null\r\n");
        }
    }
    else if (cmd_eq(cmd, "ap_ip")) {
        snprintf(resp, sizeof(resp), "ap_ip:192.168.4.1\r\n");
    }
    else if (cmd_eq(cmd, "wifi_ver")) {
        snprintf(resp, sizeof(resp), "NanoCam Board Ver:%s\r\n", VERSION);
    }
    else if (cmd_eq(cmd, "ai_mode")) {
        snprintf(resp, sizeof(resp), "\r\n>>> OK ai_mode=%d\r\n", s_ai_mode);
    }
    else if (cmd_eq(cmd, "ros_ip")) {
        snprintf(resp, sizeof(resp), "\r\n>>> OK ros_ip=%s\r\n", s_ros_ip[0] ? s_ros_ip : "not set");
    }
    else if (cmd_eq(cmd, "wifi_reset")) {
        nvs_erase_all_settings();
        snprintf(resp, sizeof(resp), "Reset_OK\r\n");
        nano_serial_send((uint8_t*)resp, strlen(resp));
        vTaskDelay(pdMS_TO_TICKS(500));
        esp_restart();
        return;
    }
    else if (cmd_is(cmd, "nano_server:")) {
        const char *val = cmd + 12;
        if (strlen(val) > 0 && strlen(val) < 128) {
            ensure_mutex();
            xSemaphoreTake(s_serial_mutex, portMAX_DELAY);
            strncpy(s_server_url, val, 127);
            xSemaphoreGive(s_serial_mutex);
            save_str("server", s_server_url);
            snprintf(resp, sizeof(resp), "OK server=%s\r\n", s_server_url);
        } else {
            snprintf(resp, sizeof(resp), "fail,invalid url\r\n");
        }
    }
    else if (cmd_is(cmd, "nano_api_key:")) {
        const char *val = cmd + 13;
        if (strlen(val) > 0 && strlen(val) < 128) {
            ensure_mutex();
            xSemaphoreTake(s_serial_mutex, portMAX_DELAY);
            strncpy(s_api_key, val, 127);
            xSemaphoreGive(s_serial_mutex);
            save_str("apikey", s_api_key);
            snprintf(resp, sizeof(resp), "OK\r\n");
        } else {
            snprintf(resp, sizeof(resp), "fail,invalid key\r\n");
        }
    }
    else if (cmd_is(cmd, "ros_ip:")) {
        const char *val = cmd + 7;
        if (strlen(val) > 0 && strlen(val) < 32) {
            ensure_mutex();
            xSemaphoreTake(s_serial_mutex, portMAX_DELAY);
            strncpy(s_ros_ip, val, 31);
            xSemaphoreGive(s_serial_mutex);
            save_str("rosip", s_ros_ip);
            snprintf(resp, sizeof(resp), "OK ros_ip=%s (saved, use nano_reboot to apply)\r\n", s_ros_ip);
        } else {
            snprintf(resp, sizeof(resp), "fail,invalid IP\r\n");
        }
    }
    else if (cmd_is(cmd, "nano_snap")) {
        static int snap_count = 0;
        char path[64];
        snap_count++;
        snprintf(path, sizeof(path), "/spiffs/photo_%04d.jpg", snap_count);
        esp_err_t err = nano_camera_snap(path);
        if (err == ESP_OK) {
            snprintf(resp, sizeof(resp), "OK %s\r\n", path);
        } else {
            snprintf(resp, sizeof(resp), "fail,snap error\r\n");
        }
    }
    else if (cmd_is(cmd, "nano_reboot")) {
        snprintf(resp, sizeof(resp), "Rebooting...\r\n");
        nano_serial_send((uint8_t*)resp, strlen(resp));
        vTaskDelay(pdMS_TO_TICKS(500));
        esp_restart();
        return;
    }
    else if (cmd_eq(cmd, "face_eril")) {
        face_recognition_send_event(ENROLL);
        snprintf(resp, sizeof(resp), "\r\n>>> face enroll triggered\r\n");
    }
    else if (cmd_eq(cmd, "face_rz")) {
        face_recognition_send_event(RECOGNIZE);
        snprintf(resp, sizeof(resp), "\r\n>>> face recognize triggered\r\n");
    }
    else if (cmd_eq(cmd, "face_del")) {
        face_recognition_send_event(DELETE);
        snprintf(resp, sizeof(resp), "\r\n>>> face delete triggered\r\n");
    }
    else if (cmd_eq(cmd, "face_detect")) {
        face_recognition_send_event(DETECT);
        snprintf(resp, sizeof(resp), "\r\n>>> face detect mode\r\n");
    }
    else if (cmd_is(cmd, "nano_info")) {
        const char *ip = s_sta_ip_cb ? s_sta_ip_cb() : "0.0.0.0";
        snprintf(resp, sizeof(resp),
            "{\"device\":\"NanoCam\",\"ver\":\"%s\",\"chip\":\"ESP32-S3\","
            "\"flash\":\"16MB\",\"psram\":\"8MB\",\"ai_mode\":%d,"
            "\"wifi_mode\":%d,\"sta_ip\":\"%s\",\"free_heap\":%lu}\r\n",
            VERSION, s_ai_mode, s_wifi_mode, ip, (unsigned long)esp_get_free_heap_size());
    }
    else {
        snprintf(resp, sizeof(resp), "\r\n>>> fail: unknown command\r\n");
    }

    nano_serial_send((uint8_t*)resp, strlen(resp));
}

static void uart_rx_task(void *arg) {
    char cmd_buf[256];
    int cmd_len = 0;
    uart_write_bytes(UART_NUM, "\r\n[NanoCam] Serial CMD ready\r\n", 29);

    while (1) {
        char c;
        int len = uart_read_bytes(UART_NUM, &c, 1, pdMS_TO_TICKS(50));
        if (len != 1) continue;

        if (c == '\r' || c == '\n') {
            if (cmd_len > 0) {
                cmd_buf[cmd_len] = '\0';
                handle_command(cmd_buf);
                cmd_len = 0;
            }
        } else if (cmd_len < (int)sizeof(cmd_buf) - 1) {
            cmd_buf[cmd_len++] = c;
        }
    }
}

esp_err_t nano_serial_init(void) {
    load_settings();

    uart_config_t uart_cfg = {
        .baud_rate  = 115200,
        .data_bits  = UART_DATA_8_BITS,
        .parity     = UART_PARITY_DISABLE,
        .stop_bits  = UART_STOP_BITS_1,
        .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    // ESP-IDF best-practice order: param_config → set_pin → driver_install
    // driver_install registers ISR; hardware must be configured first.
    ESP_ERROR_CHECK(uart_param_config(UART_NUM, &uart_cfg));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM,
        NANOCAM_UART0_TX, NANOCAM_UART0_RX,
        UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    gpio_set_pull_mode(NANOCAM_UART0_RX, GPIO_PULLUP_ONLY);
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM, 2048, 2048, 0, NULL, 0));

    xTaskCreate(uart_rx_task, "uart_rx", 5 * 1024, NULL, 15, NULL);

    vTaskDelay(pdMS_TO_TICKS(500));
    nano_serial_execute_cmd("wifi_ver");

    return ESP_OK;
}

int nano_serial_send(const uint8_t *data, uint16_t len) {
    return uart_write_bytes(UART_NUM, data, len);
}

void nano_serial_execute_cmd(const char *cmd) {
    handle_command(cmd);
}

void nano_serial_set_sta_ip_cb(const char *(*cb)(void)) {
    s_sta_ip_cb = cb;
}
