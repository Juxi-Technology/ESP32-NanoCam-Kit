#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t nano_serial_init(void);
int nano_serial_send(const uint8_t *data, uint16_t len);
void nano_serial_execute_cmd(const char *cmd);
void nano_serial_set_sta_ip_cb(const char *(*cb)(void));

int nano_serial_get_ai_mode(void);
int nano_serial_get_wifi_mode(void);
const char *nano_serial_get_sta_ssid(void);
const char *nano_serial_get_sta_pass(void);
const char *nano_serial_get_ap_ssid(void);
const char *nano_serial_get_ap_pass(void);
const char *nano_serial_get_server(void);
const char *nano_serial_get_api_key(void);
const char *nano_serial_get_ros_ip(void);

#ifdef __cplusplus
}
#endif
