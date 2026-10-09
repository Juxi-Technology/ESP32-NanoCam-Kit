#pragma once

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*nano_httpd_cmd_cb_t)(const char *cmd);

esp_err_t nano_httpd_start(QueueHandle_t frame_queue, uint16_t port, nano_httpd_cmd_cb_t cmd_cb);

void nano_httpd_stop(void);

/**
 * Returns a JSON array of recent log lines (newest last).
 * Caller must free the returned string.
 */
char *nano_httpd_get_logs(void);

#ifdef __cplusplus
}
#endif
