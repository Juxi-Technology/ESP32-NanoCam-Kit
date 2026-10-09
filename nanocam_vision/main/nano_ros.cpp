
#ifdef CONFIG_NANOCAM_ENABLE_ROS

#include "nano_ros.h"
#include "nano_config.h"

#include &lt;stdio.h&gt;
#include &lt;string.h&gt;
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include &lt;rcl/rcl.h&gt;
#include &lt;rcl/error_handling.h&gt;
#include &lt;rclc/rclc.h&gt;
#include &lt;rclc/executor.h&gt;
#include &lt;rmw_microros/rmw_microros.h&gt;
#include &lt;sensor_msgs/msg/image.h&gt;
#include &lt;std_msgs/msg/string.h&gt;
#include &lt;uros_network_interfaces.h&gt;

static const char *TAG = "nano_ros";

static rcl_allocator_t    s_allocator;
static rclc_support_t     s_support;
static rcl_node_t         s_node;
static rcl_publisher_t    s_img_pub;
static rcl_publisher_t    s_detect_pub;
static rclc_executor_t    s_executor;
static rcl_timer_t        s_timer;
static sensor_msgs__msg__Image s_img_msg;
static std_msgs__msg__String    s_detect_msg;
static bool s_initialized = false;
static bool s_connected  = false;
static char s_agent_ip[32]   = "192.168.206.130";
static uint16_t s_agent_port = 9999;

static void timer_callback(rcl_timer_t *timer, int64_t last_call_time) {
    (void)timer; (void)last_call_time;
    s_connected = rmw_uros_epoch_sync_available();
}

esp_err_t nano_ros_init(const char *agent_ip, uint16_t agent_port) {
    if (s_initialized) return ESP_ERR_INVALID_STATE;
    if (agent_ip &amp;&amp; agent_ip[0]) strncpy(s_agent_ip, agent_ip, 31);
    if (agent_port &gt; 0) s_agent_port = agent_port;
    ESP_LOGI(TAG, "ROS enabled! micro-ROS init: %s:%d", s_agent_ip, s_agent_port);

    rmw_uros_set_custom_transport(true, NULL,
        esp32_wifi_udp_open, esp32_wifi_udp_close,
        esp32_wifi_udp_write, esp32_wifi_udp_read);

    s_allocator = rcl_get_default_allocator();
    rcl_ret_t rc = rclc_support_init(&amp;s_support, 0, NULL, &amp;s_allocator);
    if (rc != RCL_RET_OK) { ESP_LOGE(TAG, "support_init fail"); return ESP_FAIL; }

    rc = rclc_node_init_default(&amp;s_node, "nanocam_node", "", &amp;s_support);
    if (rc != RCL_RET_OK) { ESP_LOGE(TAG, "node_init fail"); return ESP_FAIL; }

    rc = rclc_publisher_init_default(&amp;s_img_pub, &amp;s_node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, Image), "/esp32_img");
    if (rc != RCL_RET_OK) { ESP_LOGE(TAG, "img_pub fail"); return ESP_FAIL; }

    rc = rclc_publisher_init_default(&amp;s_detect_pub, &amp;s_node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String), "/esp32_detect");
    if (rc != RCL_RET_OK) { ESP_LOGE(TAG, "det_pub fail"); return ESP_FAIL; }

    rc = rclc_executor_init(&amp;s_executor, &amp;s_support.context, 1, &amp;s_allocator);
    if (rc != RCL_RET_OK) { ESP_LOGE(TAG, "exec init fail"); return ESP_FAIL; }

    rc = rclc_timer_init_default(&amp;s_timer, &amp;s_support, RCL_MS_TO_NS(1000), timer_callback);
    rclc_executor_add_timer(&amp;s_executor, &amp;s_timer);

    sensor_msgs__msg__Image__init(&amp;s_img_msg);
    s_img_msg.header.frame_id.data = (char*)"esp32_cam_frame";
    s_img_msg.header.frame_id.size = strlen("esp32_cam_frame");
    s_img_msg.header.frame_id.capacity = s_img_msg.header.frame_id.size + 1;
    s_img_msg.encoding.data = (char*)"jpeg";
    s_img_msg.encoding.size = 4;
    s_img_msg.encoding.capacity = 5;
    s_img_msg.is_bigendian = 0;
    s_img_msg.step = 0;

    std_msgs__msg__String__init(&amp;s_detect_msg);
    s_initialized = true;
    s_connected = true;
    ESP_LOGI(TAG, "[OK] Topics: /esp32_img, /esp32_detect");
    return ESP_OK;
}

void nano_ros_publish_image(const uint8_t *jpeg_buf, size_t buf_len,
                            uint16_t width, uint16_t height) {
    if (!s_initialized) return;
    s_img_msg.header.stamp.sec = esp_log_timestamp() / 1000;
    s_img_msg.header.stamp.nanosec = (esp_log_timestamp() % 1000) * 1000000;
    s_img_msg.height = height;
    s_img_msg.width = width;
    s_img_msg.data.data = (uint8_t*)jpeg_buf;
    s_img_msg.data.size = buf_len;
    s_img_msg.data.capacity = buf_len;
    s_img_msg.step = buf_len;
    rcl_publish(&amp;s_img_pub, &amp;s_img_msg, NULL);
    rclc_executor_spin_some(&amp;s_executor, RCL_MS_TO_NS(10));
}

void nano_ros_publish_detect(const char *json_str) {
    if (!s_initialized || !json_str) return;
    s_detect_msg.data.data = (char*)json_str;
    s_detect_msg.data.size = strlen(json_str);
    s_detect_msg.data.capacity = s_detect_msg.data.size;
    rcl_publish(&amp;s_detect_pub, &amp;s_detect_msg, NULL);
    rclc_executor_spin_some(&amp;s_executor, RCL_MS_TO_NS(10));
}

bool nano_ros_is_connected(void) {
    return s_initialized && s_connected;
}

void nano_ros_spin_some(void) {
    if (!s_initialized) return;
    rclc_executor_spin_some(&s_executor, RCL_MS_TO_NS(10));
}

#else

#include "nano_ros.h"
#include "esp_log.h"
esp_err_t nano_ros_init(const char *ip, uint16_t port) {
    ESP_LOGI("nano_ros", "ROS disabled (CONFIG_NANOCAM_ENABLE_ROS=n)");
    return ESP_OK;
}
void nano_ros_publish_image(const uint8_t *b, size_t l, uint16_t w, uint16_t h) {}
void nano_ros_publish_detect(const char *s) {}
bool nano_ros_is_connected(void) { return false; }
void nano_ros_spin_some(void) {}

#endif

