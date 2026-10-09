#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#ifdef __cplusplus
extern "C" {
#endif

void register_qr_code_scanner(const QueueHandle_t frame_i,
                              const QueueHandle_t frame_o,
                              const bool camera_fb_return);

const char *nano_qrcode_get_last_result(void);

#ifdef __cplusplus
}
#endif
