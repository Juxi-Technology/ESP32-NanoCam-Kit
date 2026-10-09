#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize micro-ROS client with UDP transport.
 *
 * @param agent_ip   micro-ROS agent IP address (e.g. "192.168.1.100")
 * @param agent_port micro-ROS agent port (default: 9999)
 * @return ESP_OK on success
 */
esp_err_t nano_ros_init(const char *agent_ip, uint16_t agent_port);

/**
 * Publish a camera frame over ROS2 topic /esp32_img.
 *
 * @param jpeg_buf   JPEG encoded image buffer
 * @param buf_len    Buffer length in bytes
 * @param width      Image width
 * @param height     Image height
 */
void nano_ros_publish_image(const uint8_t *jpeg_buf, size_t buf_len,
                            uint16_t width, uint16_t height);

/**
 * Publish AI detection result over ROS2 topic /esp32_detect.
 *
 * @param json_str   JSON formatted detection result
 *                   e.g. {"type":"face","x":100,"y":150,"w":80,"h":80,"confidence":0.95}
 */
void nano_ros_publish_detect(const char *json_str);

/**
 * Check if ROS client is connected to agent.
 */
bool nano_ros_is_connected(void);

/**
 * Spin the executor for a while to handle pending work.
 */
void nano_ros_spin_some(void);

#ifdef __cplusplus
}
#endif
