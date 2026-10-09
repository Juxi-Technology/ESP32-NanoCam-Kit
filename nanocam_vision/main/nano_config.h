#pragma once

// ============================================================
// ESP32-NanoCam 硬件引脚定义
// 基于 FMF-ESPCAM 原理图 (产品引脚配置_修正版.md)
// ============================================================

#include "driver/gpio.h"

// ---------- Camera DVP Interface ----------
#define NANOCAM_CAMERA_MODULE_NAME   "NanoCam"

#define NANOCAM_CAM_PIN_PWDN    GPIO_NUM_12   // DVP_PWDN
#define NANOCAM_CAM_PIN_RESET   GPIO_NUM_14   // DVP_RST
#define NANOCAM_CAM_PIN_XCLK    GPIO_NUM_9    // DVP_XCLK
#define NANOCAM_CAM_PIN_SIOD    GPIO_NUM_41   // SCCB SDA
#define NANOCAM_CAM_PIN_SIOC    GPIO_NUM_42   // SCCB SCL

#define NANOCAM_CAM_PIN_D7      GPIO_NUM_10   // DVP_D7
#define NANOCAM_CAM_PIN_D6      GPIO_NUM_8    // DVP_D6
#define NANOCAM_CAM_PIN_D5      GPIO_NUM_7    // DVP_D5
#define NANOCAM_CAM_PIN_D4      GPIO_NUM_5    // DVP_D4
#define NANOCAM_CAM_PIN_D3      GPIO_NUM_3    // DVP_D3
#define NANOCAM_CAM_PIN_D2      GPIO_NUM_1    // DVP_D2
#define NANOCAM_CAM_PIN_D1      GPIO_NUM_2    // DVP_D1
#define NANOCAM_CAM_PIN_D0      GPIO_NUM_4    // DVP_D0

#define NANOCAM_CAM_PIN_VSYNC   GPIO_NUM_13   // DVP_VSYNC
#define NANOCAM_CAM_PIN_HREF    GPIO_NUM_11   // DVP_HREF
#define NANOCAM_CAM_PIN_PCLK    GPIO_NUM_6    // DVP_PCLK

#define NANOCAM_CAM_XCLK_FREQ   15000000      // 15MHz (GC2145 max stable)

// ---------- Audio I2S (ES8311 Standard Duplex)
#define NANOCAM_ES8311_I2C_ADDR     0x30

#define AUDIO_I2S_GPIO_MCLK     GPIO_NUM_39   // MCLK
#define AUDIO_I2S_GPIO_WS       GPIO_NUM_37   // LRCK
#define AUDIO_I2S_GPIO_BCLK     GPIO_NUM_38   // SCLK
#define AUDIO_I2S_GPIO_DIN      GPIO_NUM_36   // DIN
#define AUDIO_I2S_GPIO_DOUT     GPIO_NUM_45   // DOUT
#define AUDIO_CODEC_PA_PIN      GPIO_NUM_NC
#define AUDIO_CODEC_I2C_SDA_PIN GPIO_NUM_41   // SDA
#define AUDIO_CODEC_I2C_SCL_PIN GPIO_NUM_42   // SCL

// ---------- SPI Flash (GD25Q128ESIG) ----------
#define NANOCAM_FLASH_CS        GPIO_NUM_29
#define NANOCAM_FLASH_SCLK      GPIO_NUM_30
#define NANOCAM_FLASH_SO        GPIO_NUM_31
#define NANOCAM_FLASH_SI        GPIO_NUM_32
#define NANOCAM_FLASH_HOLD      GPIO_NUM_33
#define NANOCAM_FLASH_WP        GPIO_NUM_34

// ---------- I2C Bus (Camera SCCB + ES8311 Codec + I2C Slave) ----------
#define NANOCAM_I2C_SDA         GPIO_NUM_41
#define NANOCAM_I2C_SCL         GPIO_NUM_42

// ---------- UART0 (USB Serial via CH340K) ----------
#define NANOCAM_UART0_TX        GPIO_NUM_43
#define NANOCAM_UART0_RX        GPIO_NUM_44

// ---------- Boot & Control ----------
#define NANOCAM_BOOT_BUTTON     GPIO_NUM_0
#define NANOCAM_CHIP_PU         GPIO_NUM_NC

// ---------- WS2812 RGB LED ----------
#define NANOCAM_RGB_LED_GPIO    GPIO_NUM_18
#define NANOCAM_RGB_LED_COUNT   1

// ---------- Onboard LED ----------
#define NANOCAM_LED_GPIO        GPIO_NUM_NC

// ---------- SD Card ----------
#define NANOCAM_SD_DETECT       GPIO_NUM_NC
