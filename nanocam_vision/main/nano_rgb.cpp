#include "nano_rgb.h"
#include "nano_config.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/rmt_tx.h"

static const char *TAG = "nano_rgb";

static rmt_channel_handle_t s_rmt_ch = NULL;
static rmt_encoder_handle_t s_encoder = NULL;

// WS2812 timing (T0H=0.35us, T0L=0.8us, T1H=0.7us, T1L=0.6us, RESET>50us)
// RMT resolution: 10MHz -> 1 tick = 0.1us
#define WS2812_T0H  4   // 0.4us
#define WS2812_T0L  8   // 0.8us
#define WS2812_T1H  8   // 0.8us
#define WS2812_T1L  5   // 0.5us
#define WS2812_RESET_TICKS 600  // >50us reset

// -------- Simple RMT encoder for WS2812 --------
typedef struct {
    rmt_encoder_t base;
    rmt_encoder_t *copy_encoder;
    uint32_t reset_ticks;
    int state;
    rmt_symbol_word_t reset_code;
} ws2812_encoder_t;

static size_t ws2812_encoder_encode(rmt_encoder_t *encoder, rmt_channel_handle_t channel,
                                     const void *primary_data, size_t data_size,
                                     rmt_encode_state_t *ret_state) {
    ws2812_encoder_t *ws = __containerof(encoder, ws2812_encoder_t, base);
    rmt_encode_state_t session_state = RMT_ENCODING_RESET;
    size_t encoded = 0;

    const rmt_symbol_word_t *symbols = (const rmt_symbol_word_t *)primary_data;
    int num_symbols = data_size / sizeof(rmt_symbol_word_t);

    if (ws->state < num_symbols) {
        encoded = ws->copy_encoder->encode(ws->copy_encoder, channel,
                                            &symbols[ws->state],
                                            (num_symbols - ws->state) * sizeof(rmt_symbol_word_t),
                                            &session_state);
        ws->state += encoded / sizeof(rmt_symbol_word_t);
        if (ws->state >= num_symbols) {
            encoded += ws->copy_encoder->encode(ws->copy_encoder, channel,
                                                 &ws->reset_code, sizeof(rmt_symbol_word_t),
                                                 &session_state);
            session_state = RMT_ENCODING_COMPLETE;
        }
    } else {
        session_state = RMT_ENCODING_COMPLETE;
    }

    *ret_state = session_state;
    return encoded;
}

static esp_err_t ws2812_encoder_del(rmt_encoder_t *encoder) {
    ws2812_encoder_t *ws = __containerof(encoder, ws2812_encoder_t, base);
    ws->copy_encoder->del(ws->copy_encoder);
    free(ws);
    return ESP_OK;
}

static esp_err_t ws2812_encoder_reset(rmt_encoder_t *encoder) {
    ws2812_encoder_t *ws = __containerof(encoder, ws2812_encoder_t, base);
    ws->copy_encoder->reset(ws->copy_encoder);
    ws->state = 0;
    return ESP_OK;
}

static esp_err_t ws2812_new_encoder(rmt_encoder_handle_t *ret_encoder) {
    ws2812_encoder_t *enc = (ws2812_encoder_t *)calloc(1, sizeof(ws2812_encoder_t));
    if (!enc) return ESP_ERR_NO_MEM;

    rmt_bytes_encoder_config_t byte_cfg = {};
    byte_cfg.bit0.level0 = 1; byte_cfg.bit0.duration0 = WS2812_T0H;
    byte_cfg.bit0.level1 = 0; byte_cfg.bit0.duration1 = WS2812_T0L;
    byte_cfg.bit1.level0 = 1; byte_cfg.bit1.duration0 = WS2812_T1H;
    byte_cfg.bit1.level1 = 0; byte_cfg.bit1.duration1 = WS2812_T1L;
    rmt_new_bytes_encoder(&byte_cfg, &enc->copy_encoder);

    enc->reset_code.level0 = 0; enc->reset_code.duration0 = WS2812_RESET_TICKS;
    enc->reset_code.level1 = 0; enc->reset_code.duration1 = 0;
    enc->state = 0;

    enc->base.encode = ws2812_encoder_encode;
    enc->base.del = ws2812_encoder_del;
    enc->base.reset = ws2812_encoder_reset;
    *ret_encoder = &enc->base;
    return ESP_OK;
}

// -------- RAW LED data buffer (1 LED = GRB byte order for WS2812) --------
static void ws2812_write_grb(uint8_t g, uint8_t r, uint8_t b) {
    if (!s_rmt_ch || !s_encoder) return;
    uint8_t data[3] = {g, r, b};  // WS2812 expects GRB order
    rmt_transmit_config_t tx_cfg = {.loop_count = 0};
    rmt_transmit(s_rmt_ch, s_encoder, data, sizeof(data), &tx_cfg);
    rmt_tx_wait_all_done(s_rmt_ch, pdMS_TO_TICKS(10));
}

// -------- Public API --------

esp_err_t nano_rgb_init(void) {
    ESP_LOGI(TAG, "Initializing WS2812 on GPIO%d", NANOCAM_RGB_LED_GPIO);

    rmt_tx_channel_config_t tx_cfg = {
        .gpio_num = NANOCAM_RGB_LED_GPIO,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000,  // 10MHz -> 0.1us per tick
        .mem_block_symbols = 48,
        .trans_queue_depth = 3,
    };
    esp_err_t err = rmt_new_tx_channel(&tx_cfg, &s_rmt_ch);
    if (err != ESP_OK) { ESP_LOGE(TAG, "RMT channel create fail"); return err; }

    err = rmt_enable(s_rmt_ch);
    if (err != ESP_OK) { ESP_LOGE(TAG, "RMT enable fail"); return err; }

    err = ws2812_new_encoder(&s_encoder);
    if (err != ESP_OK) { ESP_LOGE(TAG, "Encoder create fail"); return err; }

    nano_rgb_set(0, 0, 0);  // Turn off on init
    ESP_LOGI(TAG, "RGB LED ready");
    return ESP_OK;
}

void nano_rgb_set(uint8_t r, uint8_t g, uint8_t b) {
    ws2812_write_grb(g, r, b);
}

void nano_rgb_off(void) {
    ws2812_write_grb(0, 0, 0);
}

void nano_rgb_indicate_ok(void) {
    // Single green pulse
    nano_rgb_set(0, 255, 0);
    vTaskDelay(pdMS_TO_TICKS(300));
    nano_rgb_off();
}

void nano_rgb_indicate_wifi_connecting(void) {
    // Blue slow blink - non-blocking single flash
    nano_rgb_set(0, 0, 255);
}

void nano_rgb_indicate_error(void) {
    // Red fast blink 3 times
    for (int i = 0; i < 3; i++) {
        nano_rgb_set(255, 0, 0);
        vTaskDelay(pdMS_TO_TICKS(200));
        nano_rgb_off();
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void nano_rgb_indicate_ai_mode(int mode) {
    switch (mode) {
    case 0: nano_rgb_set(0, 255, 0); break;    // Green: Stream
    case 1: nano_rgb_set(255, 165, 0); break;   // Orange: Cat Face
    case 2: nano_rgb_set(0, 128, 255); break;   // Light Blue: Face Detect
    case 3: nano_rgb_set(255, 255, 0); break;   // Yellow: Color
    case 4: nano_rgb_set(255, 0, 255); break;   // Purple: Face ID
    case 5: nano_rgb_set(0, 255, 255); break;   // Cyan: QR Code
    case 6: nano_rgb_set(255, 255, 255); break; // White: LLM (XiaoZhi)
    case 7: nano_rgb_set(128, 0, 255); break;   // Violet: ESP-Claw
    default: nano_rgb_off(); break;
    }
}
