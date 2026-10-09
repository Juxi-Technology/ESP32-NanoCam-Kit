#pragma once

#include "esp_err.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** I2C slave address (default 0x33) */
#define NANO_I2C_SLAVE_ADDR  0x33

/** Initialize I2C slave on NanoCam pins (SDA=IO41, SCL=IO42). */
esp_err_t nano_i2c_slave_init(void);

/** Update detection result (x,y,w,h) for I2C register readout. */
void nano_i2c_slave_set_detection(int16_t x, int16_t y, int16_t w, int16_t h);

/** Update face/color ID for I2C register readout. */
void nano_i2c_slave_set_id(int16_t id);

/** Get detection data via I2C (for local use, not I2C bus). */
bool nano_i2c_slave_get_detection(int16_t *x, int16_t *y, int16_t *w, int16_t *h, int16_t *id);

#ifdef __cplusplus
}
#endif