#include "nano_camera.h"
#include "nano_config.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "freertos/task.h"
#include "img_converters.h"
#include <stdio.h>

static const char *TAG = "nano_cam";

static QueueHandle_t s_frame_queue = NULL;
static TaskHandle_t s_capture_task = NULL;
static bool s_camera_initialized = false;
static int s_capture_fail_count = 0;

// ---- Frame capture task (non-blocking, frame-skip on queue full) ----
static void nano_camera_capture_task(void *arg) {
    ESP_LOGI(TAG, "Capture task started");

    while (true) {
        camera_fb_t *fb = esp_camera_fb_get();
        if (fb) {
            s_capture_fail_count = 0;
            if (s_frame_queue) {
                if (xQueueSend(s_frame_queue, &fb, 0) != pdPASS) {
                    esp_camera_fb_return(fb);
                }
            } else {
                esp_camera_fb_return(fb);
            }
        } else {
            s_capture_fail_count++;
            if (s_capture_fail_count <= 3 || s_capture_fail_count % 10 == 0) {
                ESP_LOGW(TAG, "Camera capture failed (%d consecutive)", s_capture_fail_count);
            }
            vTaskDelay(pdMS_TO_TICKS(50));
        }
    }
}

esp_err_t nano_camera_init(pixformat_t pixel_format,
                           framesize_t frame_size,
                           uint8_t fb_count,
                           QueueHandle_t frame_queue) {
    if (s_camera_initialized) {
        ESP_LOGW(TAG, "Camera already initialized");
        return ESP_ERR_INVALID_STATE;
    }

    int w = 320, h = 240;
    switch (frame_size) {
        case FRAMESIZE_QVGA:   w = 320; h = 240;  break;
        case FRAMESIZE_VGA:    w = 640; h = 480;  break;
        case FRAMESIZE_SVGA:   w = 800; h = 600;  break;
        case FRAMESIZE_XGA:    w = 1024; h = 768; break;
        case FRAMESIZE_HD:     w = 1280; h = 720; break;
        case FRAMESIZE_SXGA:   w = 1280; h = 1024; break;
        case FRAMESIZE_UXGA:   w = 1600; h = 1200; break;
        default: break;
    }
    ESP_LOGI(TAG, "Initializing camera: %s", NANOCAM_CAMERA_MODULE_NAME);
    ESP_LOGI(TAG, "  Resolution: %dx%d, Format: %d, Buffers: %d",
             w, h, pixel_format, fb_count);

    camera_config_t config = {};
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer   = LEDC_TIMER_0;
    config.pin_d0       = NANOCAM_CAM_PIN_D0;
    config.pin_d1       = NANOCAM_CAM_PIN_D1;
    config.pin_d2       = NANOCAM_CAM_PIN_D2;
    config.pin_d3       = NANOCAM_CAM_PIN_D3;
    config.pin_d4       = NANOCAM_CAM_PIN_D4;
    config.pin_d5       = NANOCAM_CAM_PIN_D5;
    config.pin_d6       = NANOCAM_CAM_PIN_D6;
    config.pin_d7       = NANOCAM_CAM_PIN_D7;
    config.pin_xclk     = NANOCAM_CAM_PIN_XCLK;
    config.pin_pclk     = NANOCAM_CAM_PIN_PCLK;
    config.pin_vsync    = NANOCAM_CAM_PIN_VSYNC;
    config.pin_href     = NANOCAM_CAM_PIN_HREF;
    config.pin_sccb_sda = NANOCAM_CAM_PIN_SIOD;
    config.pin_sccb_scl = NANOCAM_CAM_PIN_SIOC;
    config.pin_pwdn     = NANOCAM_CAM_PIN_PWDN;
    config.pin_reset    = NANOCAM_CAM_PIN_RESET;
    config.xclk_freq_hz = NANOCAM_CAM_XCLK_FREQ;
    config.pixel_format = pixel_format;
    config.frame_size   = frame_size;
    config.jpeg_quality = 12;
    config.fb_count     = fb_count;
    config.fb_location  = CAMERA_FB_IN_PSRAM;
    config.grab_mode    = CAMERA_GRAB_WHEN_EMPTY;

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera init failed: 0x%x", err);
        return err;
    }

    s_frame_queue = frame_queue;
    if (fb_count < 4 && s_frame_queue != NULL) {
        ESP_LOGW(TAG, "fb_count(%d) < queue_depth(4) — returned frames may re-enter queue. Consider fb_count >= 4.", fb_count);
    }
    s_camera_initialized = true;

    // Sensor auto-configuration
    sensor_t *s = esp_camera_sensor_get();
    if (s) {
        ESP_LOGI(TAG, "Sensor PID: 0x%02X", s->id.PID);

        // Flip and mirror defaults based on sensor type
        if (s->id.PID == OV3660_PID || s->id.PID == OV2640_PID) {
            s->set_vflip(s, 1);
        } else if (s->id.PID == GC0308_PID) {
            s->set_hmirror(s, 0);
        } else if (s->id.PID == GC032A_PID) {
            s->set_vflip(s, 1);
        }

        // Brightness & saturation for OV3660
        if (s->id.PID == OV3660_PID) {
            s->set_brightness(s, 1);
            s->set_saturation(s, -2);
        }

        // GC2145: disable AWB for stable color output
        if (s->id.PID == GC2145_PID) {
            s->set_reg(s, 0xfe, 0xFF, 0);
            s->set_reg(s, 0x42, 0xFF, 0xfd);
        }
    }

    ESP_LOGI(TAG, "Camera sensor ready, waiting for stabilization...");
    vTaskDelay(pdMS_TO_TICKS(800));

    // Start capture task on Core 1 (leaves Core 0 for WiFi/HTTP/Protocol)
    BaseType_t ret = xTaskCreatePinnedToCore(
        nano_camera_capture_task,
        "cam_capture",
        4 * 1024,           // stack
        NULL,
        5,                  // priority
        &s_capture_task,
        1                   // Core 1
    );

    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create capture task");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Camera ready: %s", NANOCAM_CAMERA_MODULE_NAME);
    return ESP_OK;
}

sensor_t *nano_camera_sensor_get(void) {
    return esp_camera_sensor_get();
}

camera_fb_t *nano_camera_capture(void) {
    return esp_camera_fb_get();
}

esp_err_t nano_camera_snap(const char *path) {
    if (!s_camera_initialized) {
        ESP_LOGE(TAG, "Camera not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    // Grab a frame
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(TAG, "Snap: capture failed");
        return ESP_FAIL;
    }

    // Convert to JPEG if needed
    uint8_t *jpeg_buf = NULL;
    size_t jpeg_len = 0;
    if (fb->format == PIXFORMAT_JPEG) {
        jpeg_buf = fb->buf;
        jpeg_len = fb->len;
    } else if (fb->format == PIXFORMAT_RGB565) {
        if (!fmt2jpg(fb->buf, fb->len, fb->width, fb->height,
                     PIXFORMAT_RGB565, 85, &jpeg_buf, &jpeg_len)) {
            ESP_LOGE(TAG, "Snap: JPEG conversion failed");
            esp_camera_fb_return(fb);
            return ESP_FAIL;
        }
    } else {
        ESP_LOGE(TAG, "Snap: unsupported format %d", fb->format);
        esp_camera_fb_return(fb);
        return ESP_FAIL;
    }

    // Save to SPIFFS
    FILE *fp = fopen(path, "wb");
    if (!fp) {
        ESP_LOGE(TAG, "Snap: failed to open %s", path);
        if (jpeg_buf != fb->buf) free(jpeg_buf);
        esp_camera_fb_return(fb);
        return ESP_FAIL;
    }

    size_t written = fwrite(jpeg_buf, 1, jpeg_len, fp);
    fclose(fp);

    if (jpeg_buf != fb->buf) free(jpeg_buf);
    esp_camera_fb_return(fb);

    if (written != jpeg_len) {
        ESP_LOGE(TAG, "Snap: write incomplete (%u/%u)", (unsigned)written, (unsigned)jpeg_len);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Snap saved: %s (%u bytes)", path, (unsigned)jpeg_len);
    return ESP_OK;
}
