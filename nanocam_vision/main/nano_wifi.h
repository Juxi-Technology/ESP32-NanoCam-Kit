#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    NANO_WIFI_MODE_AP      = 0,
    NANO_WIFI_MODE_STA     = 1,
    NANO_WIFI_MODE_APSTA   = 2,
} nano_wifi_mode_t;

typedef struct {
    char sta_ssid[32];
    char sta_password[64];
    char ap_ssid[32];
    char ap_password[64];
    nano_wifi_mode_t mode;
} nano_wifi_config_t;

/**
 * Initialize WiFi with given configuration.
 * Blocks until STA connects or times out.
 */
esp_err_t nano_wifi_init(const nano_wifi_config_t *config);

/**
 * Get current STA IP address string.
 * Returns empty string if not connected.
 */
const char *nano_wifi_get_sta_ip(void);

/**
 * Get current AP IP address (always 192.168.4.1).
 */
const char *nano_wifi_get_ap_ip(void);

#ifdef __cplusplus
}
#endif
