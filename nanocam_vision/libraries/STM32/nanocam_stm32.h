#ifndef NANOCAM_STM32_H
#define NANOCAM_STM32_H

#include "stm32f4xx_hal.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * NanoCam STM32 HAL Driver v0.1
 *
 * Usage:
 *   NanoCam_STM32 cam(&huart1);
 *   cam.init();
 *   cam.setWiFi("MyWiFi", "password");
 *   cam.setAIMode(NANOCAM_MODE_CAT_FACE);
 *   cam.readDetection(&x, &y, &w, &h);
 */

typedef enum {
    NANOCAM_STREAM     = 0,
    NANOCAM_CAT_FACE   = 1,
    NANOCAM_HUMAN_FACE = 2,
    NANOCAM_COLOR      = 3,
    NANOCAM_FACE_ID    = 4,
    NANOCAM_QR_CODE    = 5,
    NANOCAM_LLM_AGENT  = 6,
    NANOCAM_ESPCLAW    = 7,
} NanoCamMode;

typedef struct {
    int16_t x, y, w, h;
    int16_t id;
    uint16_t area;
    uint8_t confidence;  // 0-100
} NanoCamDetection;

class NanoCam_STM32 {
public:
    NanoCam_STM32(UART_HandleTypeDef *huart) : _huart(huart) {}

    void init(void) {
        _sendCmd("wifi_ver");
        HAL_Delay(200);
    }

    void setWiFi(const char *ssid, const char *pass) {
        char cmd[64];
        snprintf(cmd, sizeof(cmd), "sta_ssid:%s", ssid);
        _sendCmd(cmd); HAL_Delay(500);
        snprintf(cmd, sizeof(cmd), "sta_pd:%s", pass);
        _sendCmd(cmd); HAL_Delay(3000);
    }

    void setWiFiMode(uint8_t mode) {
        char cmd[16];
        snprintf(cmd, sizeof(cmd), "wifi_mode:%d", mode);
        _sendCmd(cmd); HAL_Delay(3000);
    }

    void setAIMode(NanoCamMode mode) {
        char cmd[16];
        snprintf(cmd, sizeof(cmd), "ai_mode:%d", (int)mode);
        _sendCmd(cmd); HAL_Delay(3000);
    }

    HAL_StatusTypeDef getSTAIP(char *ip, uint8_t maxLen) {
        _sendCmd("sta_ip");
        char buf[64];
        _readLine(buf, sizeof(buf), 2000);
        if (strncmp(buf, "sta_ip:", 7) == 0) {
            strncpy(ip, buf + 7, maxLen);
            return HAL_OK;
        }
        return HAL_ERROR;
    }

    void factoryReset(void) {
        _sendCmd("wifi_reset");
        HAL_Delay(3000);
    }

    void reboot(void) {
        _sendCmd("nano_reboot");
        HAL_Delay(3000);
    }

    /**
     * Read and parse AI detection result from serial.
     * NanoCam outputs detection frames:
     *   $face:x,y,w,h,conf#
     *   $cat:x,y,w,h,conf#
     *   $color:x,y,w,h,id,conf#
     *   $qr:data#
     *
     * Returns HAL_OK if valid detection parsed, HAL_ERROR otherwise.
     */
    HAL_StatusTypeDef readDetection(NanoCamDetection *det) {
        char line[128];
        if (_readLine(line, sizeof(line), 100) != HAL_OK) return HAL_ERROR;

        // Skip I2C hex dumps and MCP JSON
        if (strncmp(line, "I2C:", 4) == 0) return HAL_ERROR;
        if (line[0] == '{')                return HAL_ERROR;

        char *d1 = strchr(line, '$');
        char *d2 = strchr(line, '#');
        if (!d1 || !d2 || d2 < d1) return HAL_ERROR;

        *d2 = '\0';  // terminate at '#'
        char *body = d1 + 1;  // skip '$'
        char *colon = strchr(body, ':');
        if (!colon) { *d2 = '#'; return HAL_ERROR; }

        *colon = '\0';
        char *type = body;
        char *vals = colon + 1;

        memset(det, 0, sizeof(*det));
        det->id = -1;

        if (strcmp(type, "face") == 0 || strcmp(type, "cat") == 0) {
            int conf = 0;
            int n = sscanf(vals, "%hd,%hd,%hd,%hd,%d",
                           &det->x, &det->y, &det->w, &det->h, &conf);
            if (n >= 4) {
                det->confidence = (n >= 5) ? (uint8_t)conf : 0;
                det->area = (uint16_t)(det->w * det->h);
                *d2 = '#'; return HAL_OK;
            }
        }
        else if (strcmp(type, "color") == 0) {
            int conf = 0;
            int n = sscanf(vals, "%hd,%hd,%hd,%hd,%hd,%d",
                           &det->x, &det->y, &det->w, &det->h, &det->id, &conf);
            if (n >= 5) {
                det->confidence = (n >= 6) ? (uint8_t)conf : 0;
                det->area = (uint16_t)(det->w * det->h);
                *d2 = '#'; return HAL_OK;
            }
        }
        else if (strcmp(type, "qr") == 0) {
            det->id = 0;  // QR data in vals (truncated)
            *d2 = '#'; return HAL_OK;
        }

        *d2 = '#'; return HAL_ERROR;
    }

private:
    UART_HandleTypeDef *_huart;

    void _sendCmd(const char *cmd) {
        char buf[64];
        int len = snprintf(buf, sizeof(buf), "%s\r\n", cmd);
        HAL_UART_Transmit(_huart, (uint8_t*)buf, len, 1000);
    }

    HAL_StatusTypeDef _readLine(char *buf, uint16_t maxLen, uint32_t timeout) {
        uint16_t idx = 0;
        uint32_t start = HAL_GetTick();
        while (HAL_GetTick() - start < timeout) {
            uint8_t ch;
            if (HAL_UART_Receive(_huart, &ch, 1, 10) == HAL_OK) {
                if (ch == '\r' || ch == '\n') {
                    if (idx > 0) { buf[idx] = '\0'; return HAL_OK; }
                } else if (idx < maxLen - 1) {
                    buf[idx++] = ch;
                }
            }
        }
        buf[idx] = '\0';
        return HAL_TIMEOUT;
    }
};

#endif
