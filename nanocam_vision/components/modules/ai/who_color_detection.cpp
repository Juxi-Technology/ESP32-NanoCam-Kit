#include "who_color_detection.hpp"
#include "esp_camera.h"
#include "esp_log.h"
#include <cstring>
#include "fb_gfx.h"

#include "color_detector.hpp"

using namespace dl;

static const char *TAG = "color";

static QueueHandle_t xQueueFrameI = NULL;
static QueueHandle_t xQueueFrameO = NULL;
static QueueHandle_t xQueueEvent = NULL;

// Standard HSV ranges (OpenCV 0-180 scale), bgr=false.
// The library reads camera RGB565 as-is, no channel swap.
struct ColorDef { uint8_t h_lo, h_hi, s_lo, s_hi, v_lo, v_hi; int area_min; uint16_t lcd; const char *name; };
static const ColorDef g_colors[] = {
    {  0,  15,  70, 255,  90, 255, 64, 0xF800, "red"},
    { 23,  33,  70, 255,  90, 255, 64, 0xFFE0, "yellow"},
    { 34,  75,  70, 255,  90, 255, 64, 0x07E0, "green"},
    { 97, 124,  70, 255,  90, 255, 64, 0x001F, "blue"},
    {125, 155,  70, 255,  90, 255, 64, 0xF81F, "purple"},
    {  0, 180,   0,  40, 200, 255, 80, 0xFFFF, "white"},
    {  0, 180,   0, 255,   0,  50, 80, 0x0000, "black"},
};
#define NUM_COLORS (sizeof(g_colors) / sizeof(g_colors[0]))

static void draw_rect(camera_fb_t *fb, int x1, int y1, int x2, int y2, uint16_t c) {
    if (x1 > x2) { int t = x1; x1 = x2; x2 = t; }
    if (y1 > y2) { int t = y1; y1 = y2; y2 = t; }
    if (x1 < 0) x1 = 0;
    if (y1 < 0) y1 = 0;
    if (x2 >= fb->width) x2 = fb->width - 1;
    if (y2 >= fb->height) y2 = fb->height - 1;
    int w = x2 - x1;
    int h = y2 - y1;
    if (w <= 0 || h <= 0) return;
    fb_gfx_drawFastHLine(fb, x1, y1, w + 1, c);
    fb_gfx_drawFastHLine(fb, x1, y2, w + 1, c);
    fb_gfx_drawFastVLine(fb, x1, y1, h + 1, c);
    fb_gfx_drawFastVLine(fb, x2, y1, h + 1, c);
}

static void task_process_handler(void *arg) {
    camera_fb_t *frame = NULL;

    ColorDetector detector;
    detector.set_bgr(false);
    detector.set_detection_shape({80, 80, 1});
    for (int i = 0; i < (int)NUM_COLORS; i++) {
        std::vector<uint8_t> thresh = {
            g_colors[i].h_lo, g_colors[i].h_hi,
            g_colors[i].s_lo, g_colors[i].s_hi,
            g_colors[i].v_lo, g_colors[i].v_hi,
        };
        detector.register_color(thresh, g_colors[i].area_min, g_colors[i].name);
    }

    while (true) {
        if (xQueueReceive(xQueueFrameI, &frame, pdMS_TO_TICKS(300)) != pdPASS) continue;

        std::vector<std::vector<color_detect_result_t>> &results =
            detector.detect((uint16_t *)frame->buf, {(int)frame->height, (int)frame->width, 3});

        for (int ci = 0; ci < (int)results.size(); ci++) {
            for (int ri = 0; ri < (int)results[ci].size(); ri++) {
                color_detect_result_t &res = results[ci][ri];
                draw_rect(frame, res.box[0], res.box[1], res.box[2], res.box[3], g_colors[ci % NUM_COLORS].lcd);
                int ly = (res.box[1] > 20) ? res.box[1] - 16 : 0;
                int lx = res.box[0] < 0 ? 0 : (res.box[0] > 270 ? 270 : res.box[0]);
                fb_gfx_print(frame, lx, ly, g_colors[ci % NUM_COLORS].lcd, g_colors[ci % NUM_COLORS].name);
            }
        }

        if (xQueueFrameO) {
            if (xQueueSend(xQueueFrameO, &frame, 0) != pdPASS) esp_camera_fb_return(frame);
        } else {
            esp_camera_fb_return(frame);
        }
    }
}

void register_color_detection(const QueueHandle_t frame_i,
                              const QueueHandle_t event,
                              const QueueHandle_t result,
                              const QueueHandle_t frame_o,
                              const bool camera_fb_return) {
    (void)event; (void)result; (void)camera_fb_return;
    xQueueFrameI = frame_i;
    xQueueFrameO = frame_o;
    if (event) xQueueEvent = event;
    xTaskCreatePinnedToCore(task_process_handler, "color", 4 * 1024, NULL, 3, NULL, 1);
}
