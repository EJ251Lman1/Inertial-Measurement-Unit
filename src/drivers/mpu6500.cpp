#include "mpu6500.h"
#include <stdio.h>
#include "hardware/gpio.h"

//Initialisation for the I2C communication, along with the MPU
uint8_t mpu6500_init(i2c_inst_t *i2c, uint sda_pin, uint scl_pin)
{
    i2c_init(i2c, 400 * 1000);
    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(sda_pin);
    gpio_pull_up(scl_pin);

    // Wake up MPU
    uint8_t wake_cmd[2] = {0x6B, 0x00};
    i2c_write_blocking(i2c, MPU_ADDR, wake_cmd, 2, false);

    uint8_t id = mpu6500_whoami(i2c);
    printf("WHO_AM_I: 0x%02X\n", id);

    return 0;
}
// Checks the MPU6500 device ID to ensure communication is working correctly.
uint8_t mpu6500_whoami(i2c_inst_t *i2c)
{
    uint8_t reg = WHO_AM_I_REG;
    uint8_t id = 0;
    i2c_write_blocking(i2c, MPU_ADDR, &reg, 1, true);
    i2c_read_blocking(i2c, MPU_ADDR, &id, 1, false);
    return id;
}

// Reads a 16-bit from the MPU6500 at the specified register.
int16_t mpu6500_read_word(i2c_inst_t *i2c, uint8_t reg) {
    uint8_t data[2];
    i2c_write_blocking(i2c, MPU_ADDR, &reg, 1, true);
    i2c_read_blocking(i2c, MPU_ADDR, data, 2, false);
    return (int16_t)((data[0] << 8) | data[1]);
}
