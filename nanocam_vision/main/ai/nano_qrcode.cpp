#include "nano_qrcode.hpp"

#include "esp_log.h"
#include "esp_camera.h"
#include "esp_code_scanner.h"

#include <cstring>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

static const char *TAG = "qrcode";

static QueueHandle_t xQueueFrameI = NULL;
static QueueHandle_t xQueueFrameO = NULL;
static SemaphoreHandle_t g_result_mutex = NULL;
static char g_last_code[128] = "";

const char *nano_qrcode_get_last_result(void) {
    if (!g_result_mutex) return NULL;
    const char *r = NULL;
    if (xSemaphoreTake(g_result_mutex, 0) == pdTRUE) {
        if (g_last_code[0]) r = g_last_code;
        xSemaphoreGive(g_result_mutex);
    }
    return r;
}

static void qrcode_task(void *arg) {
    while (true) {
        camera_fb_t *fb = NULL;
        if (xQueueReceive(xQueueFrameI, &fb, pdMS_TO_TICKS(500)) != pdPASS) continue;
        if (!fb || !fb->buf) { if (fb) esp_camera_fb_return(fb); continue; }

        // Create fresh scanner each frame — per-frame lifecycle avoids heap corruption
        esp_image_scanner_t *scn = esp_code_scanner_create();
        if (scn) {
            esp_code_scanner_config_t cfg = {};
            cfg.mode = ESP_CODE_SCANNER_MODE_FAST;
            cfg.fmt = ESP_CODE_SCANNER_IMAGE_RGB565;
            cfg.width = fb->width;
            cfg.height = fb->height;
            esp_code_scanner_set_config(scn, cfg);

            int count = esp_code_scanner_scan_image(scn, fb->buf);

            if (count > 0) {
                const esp_code_scanner_symbol_t result = esp_code_scanner_result(scn);
                const esp_code_scanner_symbol_t *sym = &result;
                while (sym) {
                    if (sym->type_name && sym->data && sym->datalen > 0) {
                        ESP_LOGI(TAG, "Decoded [%s]: %s", sym->type_name, sym->data);
                        if (g_result_mutex && xSemaphoreTake(g_result_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
                            snprintf(g_last_code, sizeof(g_last_code), "%s: %s",
                                     sym->type_name, sym->data);
                            xSemaphoreGive(g_result_mutex);
                        }
                    }
                    sym = sym->next;
                }
            }
            esp_code_scanner_destroy(scn);
        }

        // Forward frame to MJPEG
        if (xQueueFrameO) {
            if (xQueueSend(xQueueFrameO, &fb, 0) != pdPASS) esp_camera_fb_return(fb);
        } else {
            esp_camera_fb_return(fb);
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void register_qr_code_scanner(const QueueHandle_t frame_i,
                              const QueueHandle_t frame_o,
                              const bool camera_fb_return) {
    xQueueFrameI = frame_i;
    xQueueFrameO = frame_o;
    (void)camera_fb_return;
    g_result_mutex = xSemaphoreCreateMutex();

    xTaskCreatePinnedToCore(qrcode_task, "qrcode", 4 * 1024, NULL, 6, NULL, 0);
    ESP_LOGI(TAG, "QR scanner ready");
}
