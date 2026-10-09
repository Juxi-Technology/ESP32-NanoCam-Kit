/**
 * ESP32-NanoCam — Main Entry Point (Vision Edition)
 *
 * Serial AT Commands + NVS Settings + AI Mode Dispatch
 *   ai_mode:0 → MJPEG stream only
 *   ai_mode:1 → Cat Face Detection
 *   ai_mode:2 → Human Face Detection
 *   ai_mode:3 → Color Detection
 *   ai_mode:4 → Face Recognition
 *   ai_mode:5 → QR Code Scanner
 */
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "nvs_flash.h"
#include "esp_camera.h"
#include "esp_psram.h"

#include "nano_config.h"
#include "nano_camera.h"
#include "nano_wifi.h"
#include "nano_httpd.h"
#include "nano_serial.h"
#include "nano_ros.h"
#include "nano_mqtt.h"
#include "nano_i2c_slave.h"
#include "nano_rgb.h"

#include "nano_cat_face_detection.hpp"
#include "nano_human_face_detection.hpp"
#include "nano_color_detection.hpp"
#include "nano_human_face_recognition.hpp"
#include "nano_qrcode.hpp"

#ifndef CONFIG_NANOCAM_DEFAULT_AI_MODE
#define CONFIG_NANOCAM_DEFAULT_AI_MODE 0
#endif

static const char *TAG = "nanocam";
static const char *VERSION = "0.3.0";

static void micro_ros_task(void *pvParameters) {
    while (1) {
        nano_ros_spin_some();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

extern "C" void app_main(void)
{
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    uint32_t flash_size;
    esp_flash_get_size(NULL, &flash_size);

    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "  ESP32-NanoCam v%s (Vision)", VERSION);
#ifdef CONFIG_SPIRAM
    ESP_LOGI(TAG, "  Chip:  ESP32-S3 rev%d | %luMHz | %luMB Flash | %luKB PSRAM",
             chip_info.revision,
             (unsigned long)CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ,
             (unsigned long)(flash_size / (1024 * 1024)),
             (unsigned long)(esp_psram_get_size() / 1024));
#else
    ESP_LOGI(TAG, "  Chip:  ESP32-S3 rev%d | %luMHz | %luMB Flash",
             chip_info.revision,
             (unsigned long)CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ,
             (unsigned long)(flash_size / (1024 * 1024)));
#endif
    ESP_LOGI(TAG, "  Free Heap: %lu KB", (unsigned long)(esp_get_free_heap_size() / 1024));
    ESP_LOGI(TAG, "========================================");

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "First boot — erasing NVS...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "[OK] NVS Flash");

    nano_rgb_init();
    nano_rgb_set(0, 0, 50);

    nano_serial_init();
    int ai_mode      = nano_serial_get_ai_mode();
    int wifi_mode    = nano_serial_get_wifi_mode();

    ESP_LOGI(TAG, "----------------------------------------");
    ESP_LOGI(TAG, "  AI Mode: %d (%s)", ai_mode,
             ai_mode == 0 ? "Stream"  :
             ai_mode == 1 ? "CatFace" :
             ai_mode == 2 ? "FaceDet" :
             ai_mode == 3 ? "Color"   :
             ai_mode == 4 ? "FaceID"  :
             ai_mode == 5 ? "QRCode"  : "Unknown");

    ESP_LOGI(TAG, "----------------------------------------");

    nano_wifi_config_t wifi_cfg = {};
    const char *ssid = nano_serial_get_sta_ssid();
    const char *pass = nano_serial_get_sta_pass();
    const char *ap_ssid = nano_serial_get_ap_ssid();
    const char *ap_pass = nano_serial_get_ap_pass();

    if (ssid[0]) { strncpy(wifi_cfg.sta_ssid, ssid, 31); }
    else         { strncpy(wifi_cfg.sta_ssid, "NanoCam-STA", 31); }

    if (pass[0]) { strncpy(wifi_cfg.sta_password, pass, 63); }
    else         { strncpy(wifi_cfg.sta_password, "12345678", 63); }

    strncpy(wifi_cfg.ap_ssid,     ap_ssid[0] ? ap_ssid : "NanoCam-AP", 31);
    strncpy(wifi_cfg.ap_password, ap_pass[0] ? ap_pass : "12345678",    63);

    wifi_cfg.mode = (wifi_mode == 0) ? NANO_WIFI_MODE_AP :
                    (wifi_mode == 1) ? NANO_WIFI_MODE_STA :
                                       NANO_WIFI_MODE_APSTA;

    nano_rgb_indicate_wifi_connecting();
    ret = nano_wifi_init(&wifi_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "WiFi init failed");
        nano_rgb_indicate_error();
    } else {
        ESP_LOGI(TAG, "[OK] WiFi: %s",
                 wifi_mode == 0 ? "AP" :
                 wifi_mode == 1 ? "STA" : "AP+STA");
        nano_rgb_indicate_ok();
    }

    nano_serial_set_sta_ip_cb(nano_wifi_get_sta_ip);

    pixformat_t cam_fmt = PIXFORMAT_RGB565;
    framesize_t cam_fs  = FRAMESIZE_QVGA;

    QueueHandle_t xQueueCamRaw  = xQueueCreate(6, sizeof(camera_fb_t *));
    QueueHandle_t xQueueAIOut   = (ai_mode >= 1 && ai_mode <= 5) ? xQueueCreate(6, sizeof(camera_fb_t *)) : NULL;

    ESP_LOGI(TAG, "  Camera format: %s", (cam_fmt == PIXFORMAT_JPEG) ? "JPEG" : "RGB565");
    ret = nano_camera_init(cam_fmt, cam_fs, 6, xQueueCamRaw);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Camera init FAILED! Check wiring.");
    } else {
        ESP_LOGI(TAG, "[OK] Camera: %s", NANOCAM_CAMERA_MODULE_NAME);
        nano_rgb_indicate_ok();
    }

    if (ai_mode >= 1 && ai_mode <= 5 && ret == ESP_OK) {
        ESP_LOGI(TAG, "----------------------------------------");
        ESP_LOGI(TAG, "  Starting AI Mode %d...", ai_mode);

        switch (ai_mode) {
        case 1:
            ESP_LOGI(TAG, "  Cat Face Detection");
            register_cat_face_detection(xQueueCamRaw, NULL, NULL, xQueueAIOut, false);
            break;
        case 2:
            ESP_LOGI(TAG, "  Human Face Detection");
            register_human_face_detection(xQueueCamRaw, NULL, NULL, xQueueAIOut, false);
            break;
        case 3:
            ESP_LOGI(TAG, "  Color Detection");
            register_color_detection(xQueueCamRaw, NULL, NULL, xQueueAIOut, false);
            break;
        case 4: {
            ESP_LOGI(TAG, "  Face Recognition");
            QueueHandle_t xQueueFaceEvent = xQueueCreate(4, sizeof(recognizer_state_t));
            register_human_face_recognition(xQueueCamRaw, xQueueFaceEvent, NULL, xQueueAIOut, false);
            break;
        }
        case 5:
            ESP_LOGI(TAG, "  QR Code Scanner");
            register_qr_code_scanner(xQueueCamRaw, xQueueAIOut, false);
            break;
        default:
            break;
        }
        ESP_LOGI(TAG, "[OK] AI task started");
        nano_rgb_indicate_ai_mode(ai_mode);
    }

    ESP_LOGI(TAG, "----------------------------------------");
    ESP_LOGI(TAG, "  Starting micro-ROS...");
    const char *ros_ip = nano_serial_get_ros_ip();
    const char *agent_ip = ros_ip[0] ? ros_ip : "192.168.206.130";
    ret = nano_ros_init(agent_ip, 9999);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "micro-ROS init failed (agent not running?)");
    } else {
        ESP_LOGI(TAG, "[OK] micro-ROS publishing:");
        ESP_LOGI(TAG, "  /esp32_img     (sensor_msgs/Image)");
        ESP_LOGI(TAG, "  /esp32_detect  (std_msgs/String)");
        xTaskCreatePinnedToCore(micro_ros_task, "micro_ros", 4096, NULL, 5, NULL, 1);
        ESP_LOGI(TAG, "[OK] micro-ROS spin task started");
    }

    ESP_LOGI(TAG, "----------------------------------------");

#ifdef CONFIG_NANOCAM_ENABLE_MQTT
    ESP_LOGI(TAG, "  MQTT: enabled");
    nano_mqtt_init(NULL, "nanocam", "nanocam/events");
#endif
#ifdef CONFIG_NANOCAM_ENABLE_I2C_SLAVE
    nano_i2c_slave_init();
#endif

    QueueHandle_t stream_queue = (xQueueAIOut) ? xQueueAIOut : xQueueCamRaw;
    ret = nano_httpd_start(stream_queue, 80, nano_serial_execute_cmd);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "HTTP server start failed");
    } else {
        ESP_LOGI(TAG, "[OK] HTTP server :80");
    }

    const char *ip = nano_wifi_get_sta_ip();
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "  NanoCam v%s — AI Mode %d", VERSION, ai_mode);
    ESP_LOGI(TAG, "  Serial: 115200 8N1 — type 'ai_mode:1' to switch");
    ESP_LOGI(TAG, "  http://%s/", ip[0] ? ip : "192.168.4.1");
    ESP_LOGI(TAG, "========================================");

    char ver_msg[64];
    snprintf(ver_msg, sizeof(ver_msg), "NanoCam Board Ver:%s\r\n", VERSION);
    nano_serial_send((uint8_t*)ver_msg, strlen(ver_msg));

    while (1) {
        ESP_LOGI(TAG, "[stats] Free: %lu KB | Min: %lu KB | AI: %d",
                 (unsigned long)(esp_get_free_heap_size() / 1024),
                 (unsigned long)(esp_get_minimum_free_heap_size() / 1024),
                 ai_mode);
        vTaskDelay(pdMS_TO_TICKS(30000));
    }
}
