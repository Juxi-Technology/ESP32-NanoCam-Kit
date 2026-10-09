#include "nano_wifi.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_mac.h"
#include "esp_event.h"
#include "esp_log.h"
#include "lwip/ip4_addr.h"

static const char *TAG = "nano_wifi";

static EventGroupHandle_t s_wifi_event_group = NULL;
static char s_sta_ip[16] = {0};
static char s_ap_ip[16]  = "192.168.4.1";

#define WIFI_CONNECTED_BIT  BIT0
#define WIFI_FAIL_BIT       BIT1
#define WIFI_MAX_RETRY      5

static int s_retry_count = 0;
static bool s_event_loop_created = false;

// ---- Event Handlers ----

static void wifi_event_handler(void *arg,
                               esp_event_base_t event_base,
                               int32_t event_id,
                               void *event_data) {
    switch (event_id) {
    case WIFI_EVENT_STA_START:
        ESP_LOGI(TAG, "STA started, connecting...");
        esp_wifi_connect();
        break;

    case WIFI_EVENT_STA_DISCONNECTED:
        if (s_retry_count < WIFI_MAX_RETRY) {
            esp_wifi_connect();
            s_retry_count++;
            ESP_LOGW(TAG, "STA disconnected, retry %d/%d", s_retry_count, WIFI_MAX_RETRY);
        } else {
            ESP_LOGE(TAG, "STA connection failed after %d retries", WIFI_MAX_RETRY);
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        break;

    case WIFI_EVENT_AP_STACONNECTED: {
        wifi_event_ap_staconnected_t *ev = (wifi_event_ap_staconnected_t *)event_data;
        ESP_LOGI(TAG, "AP client connected: " MACSTR, MAC2STR(ev->mac));
        break;
    }

    case WIFI_EVENT_AP_STADISCONNECTED: {
        wifi_event_ap_stadisconnected_t *ev = (wifi_event_ap_stadisconnected_t *)event_data;
        ESP_LOGI(TAG, "AP client disconnected: " MACSTR, MAC2STR(ev->mac));
        break;
    }

    default:
        break;
    }
}

static void ip_event_handler(void *arg,
                             esp_event_base_t event_base,
                             int32_t event_id,
                             void *event_data) {
    if (event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        snprintf(s_sta_ip, sizeof(s_sta_ip),
                 IPSTR, IP2STR(&event->ip_info.ip));
        ESP_LOGI(TAG, "STA Got IP: %s", s_sta_ip);
        s_retry_count = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

// ---- Public API ----

esp_err_t nano_wifi_init(const nano_wifi_config_t *config) {
    ESP_LOGI(TAG, "Initializing WiFi...");
    ESP_LOGI(TAG, "  Mode: %s",
             config->mode == NANO_WIFI_MODE_AP    ? "AP" :
             config->mode == NANO_WIFI_MODE_STA   ? "STA" : "AP+STA");

    // Initialize TCP/IP stack
    ESP_ERROR_CHECK(esp_netif_init());
    if (!s_event_loop_created) {
        esp_err_t loop_err = esp_event_loop_create_default();
        if (loop_err == ESP_ERR_INVALID_STATE) {
            ESP_LOGI(TAG, "Event loop already created");
        } else {
            ESP_ERROR_CHECK(loop_err);
        }
        s_event_loop_created = true;
    }

    s_wifi_event_group = xEventGroupCreate();

    // Register event handlers
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                                &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                                &ip_event_handler, NULL));

    // WiFi init
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Set mode
    wifi_mode_t mode;
    switch (config->mode) {
    case NANO_WIFI_MODE_AP:    mode = WIFI_MODE_AP;    break;
    case NANO_WIFI_MODE_STA:   mode = WIFI_MODE_STA;   break;
    case NANO_WIFI_MODE_APSTA: mode = WIFI_MODE_APSTA; break;
    default:                   mode = WIFI_MODE_APSTA; break;
    }
    ESP_ERROR_CHECK(esp_wifi_set_mode(mode));

    // ---- STA Config ----
    if (mode & WIFI_MODE_STA) {
        esp_netif_create_default_wifi_sta();

        wifi_config_t sta_cfg = {};
        strncpy((char *)sta_cfg.sta.ssid, config->sta_ssid, 32);
        strncpy((char *)sta_cfg.sta.password, config->sta_password, 64);
        sta_cfg.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_cfg));
        ESP_LOGI(TAG, "  STA SSID: %s", config->sta_ssid);
    }

    // ---- AP Config ----
    if (mode & WIFI_MODE_AP) {
        esp_netif_t *ap_netif = esp_netif_create_default_wifi_ap();

        // Set AP IP to 192.168.4.1
        esp_netif_ip_info_t ip_info;
        IP4_ADDR(&ip_info.ip, 192, 168, 4, 1);
        IP4_ADDR(&ip_info.gw, 192, 168, 4, 1);
        IP4_ADDR(&ip_info.netmask, 255, 255, 255, 0);
        esp_netif_dhcps_stop(ap_netif);
        esp_netif_set_ip_info(ap_netif, &ip_info);
        esp_netif_dhcps_start(ap_netif);

        wifi_config_t ap_cfg = {};
        strncpy((char *)ap_cfg.ap.ssid, config->ap_ssid, 32);
        strncpy((char *)ap_cfg.ap.password, config->ap_password, 64);
        ap_cfg.ap.ssid_len = strlen(config->ap_ssid);
        ap_cfg.ap.channel   = 1;
        ap_cfg.ap.max_connection = 4;
        ap_cfg.ap.authmode = (strlen(config->ap_password) > 0)
                             ? WIFI_AUTH_WPA2_PSK
                             : WIFI_AUTH_OPEN;
        ap_cfg.ap.beacon_interval = 100;

        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_cfg));
        ESP_LOGI(TAG, "  AP SSID: %s", config->ap_ssid);
    }

    // Start WiFi
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));

    // Wait for STA connection (if STA mode)
    if (mode & WIFI_MODE_STA) {
        ESP_LOGI(TAG, "Waiting for STA connection...");
        EventBits_t bits = xEventGroupWaitBits(
            s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            pdMS_TO_TICKS(30000)  // 30s timeout
        );

        if (bits & WIFI_CONNECTED_BIT) {
            ESP_LOGI(TAG, "WiFi connected! STA IP: %s", s_sta_ip);
        } else {
            ESP_LOGW(TAG, "WiFi STA connection timeout, AP mode still available");
        }
    }

    ESP_LOGI(TAG, "WiFi ready. AP IP: %s", s_ap_ip);
    return ESP_OK;
}

const char *nano_wifi_get_sta_ip(void) {
    return s_sta_ip;
}

const char *nano_wifi_get_ap_ip(void) {
    return s_ap_ip;
}
