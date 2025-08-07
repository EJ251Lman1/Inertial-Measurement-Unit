
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "drivers/dht11-pico.h"
#include "drivers/mpu6500.h"
#include "drivers/BloodOxygen_S_Pico.h"
#include "drivers/uart.h"
#include <iostream>

// I2C setup
#define I2C_PORT i2c0
#define SDA_PIN 16
#define SCL_PIN 17

// Pin for DHT11 sensor signal and its outputs
const int8_t DHT_PIN = 19;
float temperature, rel_humidity;

// MAX301002 Blood Oxygen Sensor setup
float SPO2, heartbeat;
#define MAX301002_addr 0x57

// Bluetooth UART setup for transmission
volatile bool input_ready = false;
volatile char buffer[100] = {};
volatile unsigned int ind = 0;        

// Time delays for loop iterations and sensor readings
const int middle_moop_delay = 1000, end_loop_delay = 1000;

int main() {
    stdio_init_all();

    // Setups the uart on the RP2040, getting it ready for comms over bluetooth
    initialize_uart();

    //Attempts to connect to the bluetooth master, the raspberry pi
    initialize_bluetooth();

    // Initialize MPU6500, the Accelerometer plus does a WHO_AM_I check to ensure I2C communication is working
    mpu6500_init(I2C_PORT, SDA_PIN, SCL_PIN);
 
    //Initialise the DHT11, the temperature and humidity sensor
    Dht11 dht11_sensor(DHT_PIN);
    sleep_ms(100);

    // Initialize I2C for BloodOxygenSensor, runs off same I2C bus as MPU6500
    BloodOxygenSensor sensor(I2C_PORT, MAX301002_addr);
    sleep_ms(100);
    
   
    while (1) {
        // --- Read DHT11 Sensor ---
        temperature = dht11_sensor.readT();
        sleep_ms(middle_moop_delay);
        rel_humidity = dht11_sensor.readRH();
        sleep_ms(middle_moop_delay);
        

        // --- Read Accelerometer ---
        int16_t ax = mpu6500_read_word(I2C_PORT, ACCEL_XOUT_H);
        int16_t ay = mpu6500_read_word(I2C_PORT, ACCEL_XOUT_H + 2);
        int16_t az = mpu6500_read_word(I2C_PORT, ACCEL_XOUT_H + 4);
        // Convert accelerometer readings to readable units (g's)
        float ax_g = ax / ACCEL_SENSITIVITY;
        float ay_g = ay / ACCEL_SENSITIVITY;
        float az_g = az / ACCEL_SENSITIVITY;

        // --- Read Gyroscope ---
        int16_t gx = mpu6500_read_word(I2C_PORT, GYRO_XOUT_H);
        int16_t gy = mpu6500_read_word(I2C_PORT, GYRO_XOUT_H + 2);
        int16_t gz = mpu6500_read_word(I2C_PORT, GYRO_XOUT_H + 4);
        // Convert gyroscope readings to readable units (degrees per seconds)
        float gx_dps = gx / GYRO_SENSITIVITY;
        float gy_dps = gy / GYRO_SENSITIVITY;
        float gz_dps = gz / GYRO_SENSITIVITY;

        // --- Read Blood Oxygen Sensor ---
        sensor.begin();
        sensor.sensorStartCollect();
        sleep_ms(middle_moop_delay);
        sensor.getHeartbeatSPO2();
        HeartbeatSPO2 values = sensor.getValues();
        SPO2 = values.SPO2;
        heartbeat = values.Heartbeat;
        
        //Add all sensor data to an array for easy printing and transmission
        float sensor_data[10] = 
        {
            temperature,
            rel_humidity,
            SPO2,
            heartbeat, 
            ax_g, 
            ay_g, 
            az_g,
            gx_dps,
            gy_dps,
            gz_dps
            };
            
        //print data in array plainly for debugging
        printf("%.2f %.2f %% %d %d %.2f %.2f %.2f %.2f %.2f %.2f\n",
            sensor_data[0],
            sensor_data[1],
            sensor_data[2], 
            sensor_data[3],
            sensor_data[4],
            sensor_data[5],
            sensor_data[6],
            sensor_data[7],
            sensor_data[8],
            sensor_data[9]);

        //End of the data collection
        sleep_ms(end_loop_delay);

        //Send data over bluetooth, each sensor reading is sent via sensor_data array
        for (int i = 0; i < 10; i++) {
            char buffer[50];
            sprintf(buffer, "%.2f ", sensor_data[i]);
            uart_puts(UART_ID, buffer);
        }
}}
