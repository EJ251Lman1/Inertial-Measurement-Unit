// #ifndef MPU6500_H
// #define MPU6500_H

#include "hardware/i2c.h"
#include <stdint.h>   // for uint8_t, int16_t
#include "pico/stdlib.h"  // for uint

#define MPU_ADDR 0x68
#define WHO_AM_I_REG 0x75
#define ACCEL_XOUT_H 0x3B
#define GYRO_XOUT_H 0x43
#define GYRO_SENSITIVITY 131.0f

// #ifdef __cplusplus
// extern "C" {
// #endif

uint8_t mpu6500_init(i2c_inst_t *i2c, uint sda_pin, uint scl_pin);
uint8_t mpu6500_whoami(i2c_inst_t *i2c);
int16_t mpu6500_read_word(i2c_inst_t *i2c, uint8_t reg);

// #ifdef __cplusplus
// }
// #endif

// #endif // MPU6500_H
