#pragma once

#include "esp_err.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize WS2812 RGB LED on the configured pin (GPIO18).
 * Uses RMT peripheral for precise timing.
 */
esp_err_t nano_rgb_init(void);

/**
 * Set RGB color (0-255 per channel).
 */
void nano_rgb_set(uint8_t r, uint8_t g, uint8_t b);

/**
 * Turn off the LED.
 */
void nano_rgb_off(void);

/**
 * LED status indicator patterns.
 */
void nano_rgb_indicate_ok(void);       // Green breath
void nano_rgb_indicate_wifi_connecting(void);  // Blue flash
void nano_rgb_indicate_error(void);    // Red fast blink
void nano_rgb_indicate_ai_mode(int mode);  // Color by mode

#ifdef __cplusplus
}
#endif
