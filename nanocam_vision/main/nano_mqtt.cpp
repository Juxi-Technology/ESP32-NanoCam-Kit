#ifdef CONFIG_NANOCAM_ENABLE_MQTT

#include "nano_mqtt.h"
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "mqtt_client.h"

static const char *TAG = "nano_mqtt";

static esp_mqtt_client_handle_t s_client = NULL;
static char s_topic[128] = "nanocam/events";
static bool s_connected = false;

static void mqtt_event_handler(void *arg, esp_event_base_t base,
                               int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t ev = (esp_mqtt_event_handle_t)event_data;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "Connected to broker");
        s_connected = true;
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGW(TAG, "Disconnected from broker");
        s_connected = false;
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGE(TAG, "MQTT error");
        s_connected = false;
        break;
    default:
        break;
    }
}

esp_err_t nano_mqtt_init(const char *broker_uri, const char *client_id, const char *topic) {
    if (s_client) {
        ESP_LOGW(TAG, "Already initialized");
        return ESP_ERR_INVALID_STATE;
    }
    if (topic && topic[0]) {
        strncpy(s_topic, topic, sizeof(s_topic) - 1);
    }

    esp_mqtt_client_config_t cfg = {};
    cfg.broker.address.uri = broker_uri;
    if (client_id) {
        cfg.credentials.client_id = client_id;
    }

    s_client = esp_mqtt_client_init(&cfg);
    if (!s_client) {
        ESP_LOGE(TAG, "Failed to init MQTT client");
        return ESP_FAIL;
    }

    esp_mqtt_client_register_event(s_client, MQTT_EVENT_ANY, mqtt_event_handler, NULL);
    esp_err_t err = esp_mqtt_client_start(s_client);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start MQTT: %d", err);
        return err;
    }

    ESP_LOGI(TAG, "MQTT: %s → %s", broker_uri, s_topic);
    return ESP_OK;
}

void nano_mqtt_publish(const char *json_str) {
    if (!s_client || !s_connected || !json_str) return;
    esp_mqtt_client_publish(s_client, s_topic, json_str, 0, 1, 0);
}

void nano_mqtt_publish_detect(const char *type, int x, int y, int w, int h, const char *extra) {
    if (!s_client || !s_connected) return;
    char buf[512];
    int len = snprintf(buf, sizeof(buf),
        "{\"type\":\"%s\",\"x\":%d,\"y\":%d,\"w\":%d,\"h\":%d", type, x, y, w, h);
    if (extra && len < (int)sizeof(buf) - 1) {
        snprintf(buf + len, sizeof(buf) - len, ",%s", extra);
    }
    strncat(buf, "}", sizeof(buf) - strlen(buf) - 1);
    esp_mqtt_client_publish(s_client, s_topic, buf, 0, 1, 0);
}

bool nano_mqtt_is_connected(void) {
    return s_connected;
}

#else

#include "nano_mqtt.h"
#include "esp_log.h"
esp_err_t nano_mqtt_init(const char *u, const char *c, const char *t) {
    ESP_LOGI("nano_mqtt", "MQTT disabled");
    return ESP_OK;
}
void nano_mqtt_publish(const char *s) {}
void nano_mqtt_publish_detect(const char *type, int x, int y, int w, int h, const char *e) {}
bool nano_mqtt_is_connected(void) { return false; }

#endif