#include "nano_httpd.h"
#include "nano_ros.h"
#include "nano_qrcode.hpp"
#include "fb_gfx.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "esp_log.h"
#include "esp_http_server.h"
#include "esp_camera.h"
#include "img_converters.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "nano_serial.h"

extern "C" const char *nano_wifi_get_sta_ip(void);

static const char *TAG = "nano_httpd";

static QueueHandle_t s_frame_queue = NULL;
static httpd_handle_t s_server = NULL;
static nano_httpd_cmd_cb_t s_cmd_cb = NULL;

// ---- Circular log buffer (shared across all ESP_LOG calls) ----
#define LOG_BUF_LINES  80
#define LOG_BUF_LINE_LEN 128
static char  s_log_buf[LOG_BUF_LINES][LOG_BUF_LINE_LEN];
static int   s_log_write = 0;
static SemaphoreHandle_t s_log_mutex = NULL;

static volatile int s_log_recursion_guard = 0;

static void log_buf_append(const char *msg) {
    if (!s_log_mutex) return;
    if (xSemaphoreTake(s_log_mutex, pdMS_TO_TICKS(10)) != pdTRUE) return;
    snprintf(s_log_buf[s_log_write % LOG_BUF_LINES], LOG_BUF_LINE_LEN, "%s", msg);
    s_log_write++;
    xSemaphoreGive(s_log_mutex);
}

char *nano_httpd_get_logs(void) {
    if (!s_log_mutex) return strdup("[]");
    xSemaphoreTake(s_log_mutex, portMAX_DELAY);
    int total = s_log_write;
    int start = (total > LOG_BUF_LINES) ? (total - LOG_BUF_LINES) : 0;
    int count = total - start;
    if (count > LOG_BUF_LINES) count = LOG_BUF_LINES;
    // Build JSON array manually (avoid malloc per line)
    int bufsize = count * (LOG_BUF_LINE_LEN + 10) + 10;
    char *out = (char *)malloc(bufsize);
    if (!out) { xSemaphoreGive(s_log_mutex); return strdup("[]"); }
    int pos = snprintf(out, bufsize, "[");
    for (int i = 0; i < count; i++) {
        int idx = (start + i) % LOG_BUF_LINES;
        if (s_log_buf[idx][0]) {
            pos += snprintf(out + pos, bufsize - pos, "%s\"%s\"",
                (i > 0 ? "," : ""), s_log_buf[idx]);
        }
    }
    snprintf(out + pos, bufsize - pos, "]");
    xSemaphoreGive(s_log_mutex);
    return out;
}

// ---- Global vprintf hook to capture all ESP_LOG output ----
static vprintf_like_t s_orig_vprintf = NULL;

static int log_capture_vprintf(const char *fmt, va_list args) {
    if (s_log_recursion_guard) {
        if (s_orig_vprintf) return s_orig_vprintf(fmt, args);
        return vprintf(fmt, args);
    }
    s_log_recursion_guard = 1;

    va_list args_copy;
    va_copy(args_copy, args);

    char line[LOG_BUF_LINE_LEN];
    int len = vsnprintf(line, sizeof(line), fmt, args);
    if (len > 0 && len < LOG_BUF_LINE_LEN) log_buf_append(line);

    int ret = 0;
    if (s_orig_vprintf) ret = s_orig_vprintf(fmt, args_copy);
    else ret = vprintf(fmt, args_copy);

    va_end(args_copy);
    s_log_recursion_guard = 0;
    return ret;
}

// ---- JPEG conversion buffer pool (pre-allocated, reused, avoids per-frame malloc/free) ----
#define JPEG_BUF_INIT_SIZE   (64 * 1024)
#define JPEG_BUF_MAX_SIZE    (256 * 1024)
static uint8_t *s_jpeg_buf = NULL;
static size_t   s_jpeg_buf_cap = 0;

static uint8_t *jpeg_buf_ensure(size_t needed) {
    if (needed <= s_jpeg_buf_cap) return s_jpeg_buf;
    size_t cap = (needed > JPEG_BUF_MAX_SIZE) ? needed : JPEG_BUF_MAX_SIZE;
    if (s_jpeg_buf) free(s_jpeg_buf);
    s_jpeg_buf = (uint8_t *)malloc(cap);
    if (s_jpeg_buf) s_jpeg_buf_cap = cap;
    return s_jpeg_buf;
}

#define PART_BOUNDARY "123456789000000000000987654321"
static const char *_STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *_STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char *_STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

// ---- MJPEG Stream Handler ----
static esp_err_t stream_handler(httpd_req_t *req) {
    camera_fb_t *fb = NULL;
    esp_err_t res = ESP_OK;
    char part_buf[64];

    if (!s_jpeg_buf) {
        s_jpeg_buf = (uint8_t *)malloc(JPEG_BUF_INIT_SIZE);
        if (s_jpeg_buf) s_jpeg_buf_cap = JPEG_BUF_INIT_SIZE;
    }

    res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if (res != ESP_OK) return res;

    while (true) {
        fb = NULL;
        if (xQueueReceive(s_frame_queue, &fb, pdMS_TO_TICKS(200)) == pdPASS) {
            camera_fb_t *newer = NULL;
            while (xQueueReceive(s_frame_queue, &newer, 0) == pdPASS) {
                esp_camera_fb_return(fb);
                fb = newer;
            }
        }
        if (!fb) {
            vTaskDelay(pdMS_TO_TICKS(50));
            continue;
        }

        bool is_converted = false;
        if (fb->format == PIXFORMAT_RGB565) {
            size_t out_len = 0;
            uint16_t img_width = fb->width;
            uint16_t img_height = fb->height;
            // Overlay QR code result on frame (Core 0, safe)
            const char *qr_text = nano_qrcode_get_last_result();
            if (qr_text) {
                fb_gfx_print(fb, 4, fb->height - 50, 0x07E0, qr_text);
            }

            uint8_t *out = jpeg_buf_ensure(JPEG_BUF_INIT_SIZE);
            if (out && fmt2jpg(fb->buf, fb->len, img_width, img_height,
                               PIXFORMAT_RGB565, 60, &out, &out_len)) {
                if (out != s_jpeg_buf) {
                    free(s_jpeg_buf);
                    s_jpeg_buf = out;
                    s_jpeg_buf_cap = out_len;
                }
                esp_camera_fb_return(fb);
                fb = NULL;
                is_converted = true;

                nano_ros_publish_image(out, out_len, img_width, img_height);
                
                res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
                if (res != ESP_OK) break;
                int part_len = snprintf(part_buf, sizeof(part_buf), _STREAM_PART, out_len);
            res = httpd_resp_send_chunk(req, part_buf, part_len);
            if (res != ESP_OK) break;
            res = httpd_resp_send_chunk(req, (const char *)out, out_len);
            if (res != ESP_OK) break;

            continue;
            } else {
                esp_camera_fb_return(fb);
                fb = NULL;
                vTaskDelay(pdMS_TO_TICKS(10));
                continue;
            }
        }

        if (!is_converted) {
            nano_ros_publish_image(fb->buf, fb->len, fb->width, fb->height);
            res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
            if (res != ESP_OK) { esp_camera_fb_return(fb); break; }
            int part_len = snprintf(part_buf, sizeof(part_buf), _STREAM_PART, fb->len);
            res = httpd_resp_send_chunk(req, part_buf, part_len);
            if (res != ESP_OK) { esp_camera_fb_return(fb); break; }
            res = httpd_resp_send_chunk(req, (const char *)fb->buf, fb->len);
            esp_camera_fb_return(fb);

            if (res != ESP_OK) {
                ESP_LOGI(TAG, "Stream client disconnected");
                break;
            }
        }

        camera_fb_t *stale = NULL;
        while (xQueueReceive(s_frame_queue, &stale, 0) == pdPASS) {
            esp_camera_fb_return(stale);
        }
    }

    return res;
}

// ---- Index Page (MJPEG only) ----
static esp_err_t index_handler(httpd_req_t *req) {
    const char *html =
        "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>NanoCam</title>"
        "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
        "<style>"
        "*{margin:0;padding:0;box-sizing:border-box}"
        "body{background:#111;display:flex;justify-content:center;align-items:center;min-height:100vh}"
        "img{max-width:100vw;max-height:100vh;object-fit:contain;cursor:zoom-in}"
        "img:active{cursor:zoom-out}"
        "</style></head><body>"
        "<img src=\"/stream\" alt=\"Stream\">"
        "</body></html>";

    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Cache-Control", "no-store, no-cache, must-revalidate");
    return httpd_resp_send(req, html, strlen(html));
}

static esp_err_t admin_handler(httpd_req_t *req) {
    const char *html =
        "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>NanoCam Info</title>"
        "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
        "<style>"
        "*{margin:0;padding:0;box-sizing:border-box}"
        "body{background:#111;font-family:system-ui,sans-serif;color:#fff;display:flex;"
        "flex-direction:column;align-items:center;justify-content:center;min-height:100vh}"
        "h1{font-size:20px;color:#0af;margin-bottom:20px}"
        "table{border-collapse:collapse;font-size:14px}"
        "td{padding:6px 16px;border-bottom:1px solid #333}"
        "td:first-child{color:#888;text-align:right;white-space:nowrap}"
        "td:last-child{color:#fff;font-family:monospace}"
        "a{color:#0af;text-decoration:none;font-size:14px;margin-top:24px;"
        "padding:8px 20px;border:1px solid #0af;border-radius:4px}"
        "a:hover{background:#0af;color:#111}"
        "</style></head><body>"
        "<h1>NanoCam v0.2.0</h1>"
        "<table id=\"tbl\"><tr><td>Loading...</td><td></td></tr></table>"
        "<a href=\"/\">\u76f4\u64ad\u753b\u9762</a>"
        "<script>"
        "fetch('/status').then(function(r){return r.json();}).then(function(j){"
        "var h='';"
        "h+='<tr><td>AI \u6a21\u5f0f</td><td>'+j.ai_mode+'</td></tr>';"
        "h+='<tr><td>STA IP</td><td>'+(j.sta_ip||'-')+'</td></tr>';"
        "h+='<tr><td>AP IP</td><td>'+(j.ap_ip||'-')+'</td></tr>';"
        "h+='<tr><td>\u7248\u672c</td><td>'+j.version+'</td></tr>';"
        "h+='<tr><td>\u82af\u7247</td><td>'+j.chip+'</td></tr>';"
        "h+='<tr><td>Flash</td><td>'+j.flash+'</td></tr>';"
        "h+='<tr><td>\u5185\u5b58</td><td>'+(j.free_heap/1024).toFixed(0)+'KB</td></tr>';"
        "document.getElementById('tbl').innerHTML=h;"
        "}).catch(function(){});"
        "</script></body></html>";

    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Cache-Control", "no-store, no-cache, must-revalidate");
    return httpd_resp_send(req, html, strlen(html));
}

// ---- Status JSON (enhanced with STA info) ----
static esp_err_t status_handler(httpd_req_t *req) {
    char sta_ip_buf[32] = "0.0.0.0";
    bool sta_ok = false;
    const char *ip = nano_wifi_get_sta_ip();
    if (ip && ip[0] && strcmp(ip, "0.0.0.0") != 0) {
        snprintf(sta_ip_buf, sizeof(sta_ip_buf), "%s", ip);
        sta_ok = true;
    }
    int ai_mode = nano_serial_get_ai_mode();
    char buf[512];
    snprintf(buf, sizeof(buf),
        "{\"device\":\"NanoCam\",\"version\":\"0.2.0\","
        "\"chip\":\"ESP32-S3\",\"flash\":\"16MB\",\"psram\":\"8MB\","
        "\"free_heap\":%lu,\"ai_mode\":%d,"
        "\"sta_ip\":\"%s\",\"sta_ok\":%s,"
        "\"ap_ip\":\"192.168.4.1\",\"status\":\"running\"}",
        (unsigned long)esp_get_free_heap_size(), ai_mode,
        sta_ip_buf, sta_ok ? "true" : "false");
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, buf, strlen(buf));
}

// ---- API command forwarder ----
static esp_err_t api_handler(httpd_req_t *req) {
    static int api_call = 0; api_call++;
    printf("\n=== API#%d URI=[%s] ===\n", api_call, req->uri);
    fflush(stdout);

    char buf[128];
    if (httpd_req_get_url_query_str(req, buf, sizeof(buf)) == ESP_OK) {
        printf("[WEB-API] query=[%s]\n", buf); fflush(stdout);
        char cmd[64];
        if (httpd_query_key_value(buf, "cmd", cmd, sizeof(cmd)) == ESP_OK) {
            printf("[WEB-API] cmd=%s\n", cmd);
            if (strncmp(cmd, "ai_mode:", 8) == 0) {
                int mode = cmd[8] - '0';
                printf("[WEB-API] AI mode switch to %d\n", mode);
                httpd_resp_set_type(req, "application/json");
                httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
                char resp[64];
                snprintf(resp, sizeof(resp), "{\"ok\":true,\"msg\":\"Rebooting to mode %d\"}", mode);
                httpd_resp_send(req, resp, strlen(resp));
                ESP_LOGI(TAG, "Web: ai_mode=%d — redirecting to serial handler", mode);
                vTaskDelay(pdMS_TO_TICKS(200));
                nano_serial_execute_cmd((char*)cmd);
                return ESP_OK;
            }
            if (strncmp(cmd, "sta:", 4) == 0) {
                printf("[WEB-API] STA config received\n");
                httpd_resp_set_type(req, "application/json");
                httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
                httpd_resp_send(req, "{\"ok\":true,\"msg\":\"Saving STA credentials\"}", 41);
                char scratch[128];
                const char *p = strchr(cmd, '|');
                int ssid_len = p ? (int)(p - cmd - 4) : (int)strlen(cmd) - 4;
                snprintf(scratch, sizeof(scratch), "sta_ssid:%.*s", ssid_len > 0 ? ssid_len : 0, cmd + 4);
                printf("[WEB-API] %s\n", scratch);
                ESP_LOGI(TAG, "Web: %s", scratch);
                vTaskDelay(pdMS_TO_TICKS(100));
                nano_serial_execute_cmd(scratch);
                if (p && p[1]) {
                    vTaskDelay(pdMS_TO_TICKS(100));
                    char pass_cmd[128];
                    snprintf(pass_cmd, sizeof(pass_cmd), "sta_pd:%s", p + 1);
                    ESP_LOGI(TAG, "Web: sta_pd saved");
                    nano_serial_execute_cmd(pass_cmd);
                }
                return ESP_OK;
            }
            if (strncmp(cmd, "ap:", 3) == 0) {
                printf("[WEB-API] AP config received\n");
                httpd_resp_set_type(req, "application/json");
                httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
                httpd_resp_send(req, "{\"ok\":true,\"msg\":\"Switching to AP mode\"}", 42);
                char scratch[128];
                const char *p = strchr(cmd, '|');
                int ssid_len = p ? (int)(p - cmd - 3) : (int)strlen(cmd) - 3;
                if (ssid_len > 0) {
                    snprintf(scratch, sizeof(scratch), "ap_ssid:%.*s", ssid_len, cmd + 3);
                    vTaskDelay(pdMS_TO_TICKS(100));
                    nano_serial_execute_cmd(scratch);
                }
                if (p && p[1]) {
                    vTaskDelay(pdMS_TO_TICKS(100));
                    char pass_cmd[128];
                    snprintf(pass_cmd, sizeof(pass_cmd), "ap_pd:%s", p + 1);
                    nano_serial_execute_cmd(pass_cmd);
                } else {
                    vTaskDelay(pdMS_TO_TICKS(100));
                    nano_serial_execute_cmd((char*)"ap_pd:12345678");
                }
                vTaskDelay(pdMS_TO_TICKS(100));
                nano_serial_execute_cmd((char*)"wifi_mode:0");
                return ESP_OK;
            }
            httpd_resp_set_type(req, "application/json");
            httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
            httpd_resp_send(req, "{\"ok\":true}", 9);
            vTaskDelay(pdMS_TO_TICKS(200));
            if (s_cmd_cb) s_cmd_cb(cmd);
            return ESP_OK;
        }
    }
    printf("[WEB-API] bad request: %s\n", buf);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, "{\"ok\":false}", 10);
}

// ---- Log endpoint ----
static esp_err_t log_handler(httpd_req_t *req) {
    char *logs = nano_httpd_get_logs();
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Cache-Control", "no-cache");
    esp_err_t ret = httpd_resp_send(req, logs, strlen(logs));
    free(logs);
    return ret;
}

// ---- Favicon ----
static esp_err_t favicon_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "image/x-icon");
    httpd_resp_set_status(req, "204 No Content");
    return httpd_resp_send(req, NULL, 0);
}

// ---- Ping endpoint (bare-minimum connectivity test) ----
static esp_err_t ping_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Cache-Control", "no-cache");
    return httpd_resp_send(req, "pong", 4);
}

// ---- Public API ----

esp_err_t nano_httpd_start(QueueHandle_t frame_queue, uint16_t port, nano_httpd_cmd_cb_t cmd_cb) {
    s_frame_queue = frame_queue;
    s_cmd_cb = cmd_cb;

    // Init log mutex and capture hook
    s_log_mutex = xSemaphoreCreateMutex();
    s_orig_vprintf = esp_log_set_vprintf(log_capture_vprintf);
    ESP_LOGI(TAG, "Log capture enabled");

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = port;
    config.max_uri_handlers = 10;
    config.lru_purge_enable = true;
    config.max_open_sockets = 7;
    config.recv_wait_timeout = 5;
    config.send_wait_timeout = 10;
    config.stack_size = 8192;
    config.max_resp_headers = 10;

    ESP_LOGI(TAG, "Starting HTTP server on port %d", port);
    esp_err_t err = httpd_start(&s_server, &config);
    if (err != ESP_OK) { ESP_LOGE(TAG, "HTTP start fail: %d", err); return err; }

    httpd_uri_t uris[] = {
        {"/",                HTTP_GET, index_handler,   NULL},
        {"/html/index.html", HTTP_GET, index_handler,   NULL},
        {"/admin",           HTTP_GET, admin_handler,   NULL},
        {"/favicon.ico",     HTTP_GET, favicon_handler, NULL},
        {"/stream",          HTTP_GET, stream_handler,  NULL},
        {"/status",          HTTP_GET, status_handler,  NULL},
        {"/api",             HTTP_GET, api_handler,     NULL},
        {"/log",             HTTP_GET, log_handler,     NULL},
        {"/ping",            HTTP_GET, ping_handler,    NULL},
    };
    for (int i = 0; i < 9; i++) httpd_register_uri_handler(s_server, &uris[i]);

    ESP_LOGI(TAG, "HTTP ready: /, /admin, /stream, /status, /api, /log, /ping");
    return ESP_OK;
}

void nano_httpd_stop(void) {
    if (s_server) {
        httpd_stop(s_server);
        s_server = NULL;
        ESP_LOGI(TAG, "HTTP server stopped");
    }
}
