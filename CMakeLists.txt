cmake_minimum_required(VERSION 3.12)
include(pico_sdk_import.cmake)

set(projname "mpu6050")

project(${projname} C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Specify an increased oscillator startup delay to work around a potential hardware issue where the 
    # clock is slow to boot.
    add_compile_definitions(PICO_XOSC_STARTUP_DELAY_MULTIPLIER=64)

pico_sdk_init()

add_executable(mpu6050)
target_sources(mpu6050
        PUBLIC
        src/main.cpp
        src/drivers/dht11-pico.cpp       
        src/drivers/mpu6500.cpp
        src/drivers/BloodOxygen_S_Pico.cpp
        src/drivers/uart.cpp
        )
target_include_directories(mpu6050
        PUBLIC 
        src/
        drivers/
    )
#include(example_auto_set_url.cmake)

pico_set_program_name(mpu6050 "mpu6050")
pico_set_program_version(mpu6050 "0.2")

# # select UART for standard IO
# pico_enable_stdio_uart(mpu6050 1)
# pico_enable_stdio_usb(mpu6050 0)

# Add the standard library to the build
target_link_libraries(mpu6050
pico_stdlib
hardware_spi
hardware_i2c
hardware_dma
hardware_pio
hardware_interp
hardware_timer
hardware_watchdog
hardware_clocks
hardware_pwm
hardware_adc

)

pico_add_extra_outputs(mpu6050)
