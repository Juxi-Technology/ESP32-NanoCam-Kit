#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_camera.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize camera with NanoCam pin configuration.
 *
 * @param pixel_format  PIXFORMAT_RGB565, PIXFORMAT_JPEG, etc.
 * @param frame_size    FRAMESIZE_QVGA, FRAMESIZE_VGA, etc.
 * @param fb_count      Number of frame buffers (1 or 2).
 * @param frame_queue   Queue to send captured frames to (can be NULL).
 * @return ESP_OK on success.
 */
esp_err_t nano_camera_init(pixformat_t pixel_format,
                           framesize_t frame_size,
                           uint8_t fb_count,
                           QueueHandle_t frame_queue);

/**
 * Get camera sensor handle for advanced configuration.
 */
sensor_t *nano_camera_sensor_get(void);

/**
 * Capture a single frame. Caller must return the frame with esp_camera_fb_return().
 */
camera_fb_t *nano_camera_capture(void);

/**
 * Take a snapshot and save to SPIFFS as JPEG.
 * @param path  Output path (e.g. "/spiffs/photo_001.jpg"), must be <= 64 chars.
 * @return ESP_OK on success.
 */
esp_err_t nano_camera_snap(const char *path);

#ifdef __cplusplus
}
#endif
