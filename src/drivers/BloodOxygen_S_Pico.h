#ifndef BLOODOXYGEN_S_PICO_HPP
#define BLOODOXYGEN_S_PICO_HPP

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

typedef struct {
    int SPO2;
    int Heartbeat;
} HeartbeatSPO2;

class BloodOxygenSensor{
public:
    BloodOxygenSensor(i2c_inst_t* i2c, uint8_t addr);
    bool begin();
    void getHeartbeatSPO2();
    float getTemperature_C();
    void sensorStartCollect();
    void sensorEndCollect();

    HeartbeatSPO2 getValues() const { return data; }

private:
    void writeReg(uint8_t reg_addr, uint8_t* data_buf, uint8_t len);
    int16_t readReg(uint8_t reg_addr, uint8_t* data_buf, uint8_t len);

    i2c_inst_t* _i2c;
    uint8_t _addr;
    HeartbeatSPO2 data;
};

#endif
