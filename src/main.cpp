
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "drivers/dht11-pico.h"
#include "drivers/mpu6500.h"
#include "drivers/BloodOxygen_S_Pico.h"
#include "drivers/uart.h"
#include<iostream>

#define I2C_PORT i2c0
#define SDA_PIN 16
#define SCL_PIN 17
const uint DHT_PIN = 19;
int cat = 0;


volatile bool input_ready = false;
volatile char buffer[100] = {};
volatile unsigned int ind = 0;        


int main() {
    stdio_init_all();

    initialize_uart();
    initialize_bluetooth();

    uart_init(UART_ID, BAUD_RATE);

    // Set the TX and RX pins
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    printf("WHO_AM_I");
    // Initialize MPU6500
    mpu6500_init(I2C_PORT, SDA_PIN, SCL_PIN);

    // Check WHO_AM_I register
    uint8_t id = mpu6500_whoami(I2C_PORT);
    printf("WHO_AM_I: 0x%02X\n", id);

  

    float angle_x = 0.0f, angle_y = 0.0f, angle_z = 0.0f;
    const float dt = 0.5f;
    const int loop_delay_ms = 500;

    Dht11 dht11_sensor(DHT_PIN);
    sleep_ms(100);
    double temperature;
    double rel_humidity;
    int SPO2;
    int heartbeat;
    while (1) {

        std::cout<<"Temp:"<<dht11_sensor.readT()<<" °C"<<std::endl;
        sleep_ms(1000);
         std::cout<<"RH:"<<dht11_sensor.readRH()<<" %"<<std::endl;
        sleep_ms(1000);
        dht11_sensor.readRHT(&temperature, &rel_humidity);
        sleep_ms(100);
        std::cout<<"Temp:"<<temperature<<"°C"<<"   RH:"<<rel_humidity<<" %"<<std::endl;
        sleep_ms(1000);

        // --- Read Accelerometer ---
        int16_t ax = mpu6500_read_word(I2C_PORT, ACCEL_XOUT_H);
        int16_t ay = mpu6500_read_word(I2C_PORT, ACCEL_XOUT_H + 2);
        int16_t az = mpu6500_read_word(I2C_PORT, ACCEL_XOUT_H + 4);

        float ax_g = ax / 16384.0f;
        float ay_g = ay / 16384.0f;
        float az_g = az / 16384.0f;

        // --- Read Gyroscope ---
        int16_t gx = mpu6500_read_word(I2C_PORT, GYRO_XOUT_H);
        int16_t gy = mpu6500_read_word(I2C_PORT, GYRO_XOUT_H + 2);
        int16_t gz = mpu6500_read_word(I2C_PORT, GYRO_XOUT_H + 4);

        float gx_dps = gx / GYRO_SENSITIVITY;
        float gy_dps = gy / GYRO_SENSITIVITY;
        float gz_dps = gz / GYRO_SENSITIVITY;

        // Integrate to angles
        angle_x += gx_dps * dt;
        angle_y += gy_dps * dt;
        angle_z += gz_dps * dt;

        // --- Read Temperature and Humidity ---
        

        // --- Output Sensor Data ---
        
        
        // sleep_ms(1000);
        // printf("Accel (g): X=%.2f Y=%.2f Z=%.2f\n", ax_g, ay_g, az_g);
        // sleep_ms(1000);
        // printf("Gyro (°/s): X=%.2f Y=%.2f Z=%.2f\n", gx_dps, gy_dps, gz_dps);
        // sleep_ms(1000);
        // printf("Integrated Angles (°): X=%.2f Y=%.2f Z=%.2f\n\n", angle_x, angle_y, angle_z);
        // sleep_ms(1000);
        // sleep_ms(loop_delay_ms);
        //put all values into array for transmission
        BloodOxygenSensor sensor(I2C_PORT, 0x57);
        sensor.begin();
        sensor.sensorStartCollect();
            sleep_ms(1000);
            sensor.getHeartbeatSPO2();
            HeartbeatSPO2 values = sensor.getValues();
        SPO2 = values.SPO2;
        heartbeat = values.Heartbeat;
        float sensor_data[10] = {temperature,rel_humidity,SPO2,heartbeat, ax_g, ay_g, az_g,gx_dps,gy_dps,gz_dps};
        //print data in array plainly
        printf("%.2f %.2f %% %d %d %.2f %.2f %.2f %.2f %.2f %.2f\n",
               sensor_data[0], sensor_data[1], (int)sensor_data[2], (int)sensor_data[3],
               sensor_data[4], sensor_data[5], sensor_data[6],sensor_data[7],sensor_data[8],sensor_data[9]);
        sleep_ms(1000);
        //send data over bluetooth
        for (int i = 0; i < 10; i++) {
            char buffer[50];
            sprintf(buffer, "%.2f ", sensor_data[i]);
            uart_puts(UART_ID, buffer);
        }
}}
