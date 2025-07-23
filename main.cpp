#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <iostream>
#include "dht11-pico.h"

#define I2C_PORT i2c0
#define SDA_PIN 16
#define SCL_PIN 17
#define MPU_ADDR 0x68
#define WHO_AM_I_REG 0x75
#define ACCEL_XOUT_H 0x3B
#define GYRO_XOUT_H 0x43
#define GYRO_SENSITIVITY 131.0f  // for ±250 dps

const uint DHT_PIN = 19;

// Function to read 16-bit signed value from MPU
int16_t read_word(uint8_t reg) {
    uint8_t data[2];
    i2c_write_blocking(I2C_PORT, MPU_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, MPU_ADDR, data, 2, false);
    return (int16_t)((data[0] << 8) | data[1]);
}

int main() {
    stdio_init_all();
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    sleep_ms(500);

    // WHO_AM_I check
    uint8_t reg = WHO_AM_I_REG;
    uint8_t id = 0;
    i2c_write_blocking(I2C_PORT, MPU_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, MPU_ADDR, &id, 1, false);
    printf("WHO_AM_I: 0x%02X\n", id);

    // Wake up MPU
    uint8_t wake_cmd[2] = {0x6B, 0x00};
    i2c_write_blocking(I2C_PORT, MPU_ADDR, wake_cmd, 2, false);

    Dht11 dht11_sensor(DHT_PIN);
    double temperature, rel_humidity;

    // Angle accumulators
    float angle_x = 0.0f;
    float angle_y = 0.0f;
    float angle_z = 0.0f;

    // Loop timing
    const float dt = 0.5f;  // 500 ms = 0.5 s
    const int loop_delay_ms = 500;

    while (1) {
        // --- Read Accelerometer ---
        int16_t ax = read_word(ACCEL_XOUT_H);
        int16_t ay = read_word(ACCEL_XOUT_H + 2);
        int16_t az = read_word(ACCEL_XOUT_H + 4);

        float ax_g = ax / 16384.0f;
        float ay_g = ay / 16384.0f;
        float az_g = az / 16384.0f;

        // --- Read Gyroscope ---
        int16_t gx = read_word(GYRO_XOUT_H);
        int16_t gy = read_word(GYRO_XOUT_H + 2);
        int16_t gz = read_word(GYRO_XOUT_H + 4);

        float gx_dps = gx / GYRO_SENSITIVITY;
        float gy_dps = gy / GYRO_SENSITIVITY;
        float gz_dps = gz / GYRO_SENSITIVITY;

        // Integrate to angles
        angle_x += gx_dps * dt;
        angle_y += gy_dps * dt;
        angle_z += gz_dps * dt;

        // --- Read Temperature and Humidity ---
        std::cout << "Temp: " << dht11_sensor.readT() << " °C" << std::endl;
        sleep_ms(1000);
        std::cout << "RH: " << dht11_sensor.readRH() << " %" << std::endl;
        sleep_ms(1000);
        dht11_sensor.readRHT(&temperature, &rel_humidity);
        sleep_ms(100);
        std::cout << "Temp: " << temperature << "°C   RH: " << rel_humidity << " %" << std::endl;

        // --- Output Sensor Data ---
        printf("Accel (g): X=%.2f Y=%.2f Z=%.2f\n", ax_g, ay_g, az_g);
        printf("Gyro (°/s): X=%.2f Y=%.2f Z=%.2f\n", gx_dps, gy_dps, gz_dps);
        printf("Integrated Angles (°): X=%.2f Y=%.2f Z=%.2f\n\n", angle_x, angle_y, angle_z);

        sleep_ms(loop_delay_ms);
    }
}
