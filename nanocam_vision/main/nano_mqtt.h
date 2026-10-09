#pragma once

#include "esp_err.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize MQTT client and connect to broker.
 *
 * @param broker_uri  MQTT broker URI (e.g. "mqtt://192.168.1.100:1883")
 * @param client_id   MQTT client ID (NULL = auto-generate)
 * @param topic       Base topic for publishing (e.g. "nanocam/events")
 * @return ESP_OK on success
 */
esp_err_t nano_mqtt_init(const char *broker_uri, const char *client_id, const char *topic);

/**
 * Publish a JSON message to the configured topic.
 */
void nano_mqtt_publish(const char *json_str);

/**
 * Publish AI detection result.
 *
 * @param type    Detection type: "cat", "face", "color", "motion", "qr"
 * @param x       Bounding box X (or -1 if N/A)
 * @param y       Bounding box Y (or -1 if N/A)
 * @param w       Bounding box width
 * @param h       Bounding box height
 * @param extra   Extra JSON fields (e.g. "\"confidence\":0.95"), or NULL
 */
void nano_mqtt_publish_detect(const char *type, int x, int y, int w, int h, const char *extra);

/**
 * Check if MQTT is connected.
 */
bool nano_mqtt_is_connected(void);

#ifdef __cplusplus
}
#endif