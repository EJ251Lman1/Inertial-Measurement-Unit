#include "BloodOxygen_S_Pico.h"
#include "hardware/i2c.h"
#include "pico/time.h"

BloodOxygenSensor::BloodOxygenSensor(i2c_inst_t* i2c, uint8_t addr)
    : _i2c(i2c), _addr(addr) {}

bool BloodOxygenSensor::begin() {
    uint8_t dummy = 0x00;
    int result = i2c_write_blocking(_i2c, _addr, &dummy, 1, true);
    return result >= 0;
}

void BloodOxygenSensor::writeReg(uint8_t reg_addr, uint8_t* data_buf, uint8_t len) {
    uint8_t buffer[len + 1];
    buffer[0] = reg_addr;
    for (uint8_t i = 0; i < len; ++i) {
        buffer[i + 1] = data_buf[i];
    }
    i2c_write_blocking(_i2c, _addr, buffer, len + 1, false);
}

int16_t BloodOxygenSensor::readReg(uint8_t reg_addr, uint8_t* data_buf, uint8_t len) {
    if (i2c_write_blocking(_i2c, _addr, &reg_addr, 1, true) < 0) return -1;
    int read = i2c_read_blocking(_i2c, _addr, data_buf, len, false);
    return read;
}

void BloodOxygenSensor::getHeartbeatSPO2() {
    uint8_t buf[8];
    readReg(0x0C, buf, 8);
    data.SPO2 = buf[0];
    if (data.SPO2 == 0) data.SPO2 = -1;

    data.Heartbeat = (buf[2] << 24) | (buf[3] << 16) | (buf[4] << 8) | buf[5];
    if (data.Heartbeat == 0) data.Heartbeat = -1;
}

float BloodOxygenSensor::getTemperature_C() {
    uint8_t buf[2];
    readReg(0x14, buf, 2);
    return buf[0] + buf[1] / 100.0f;
}

void BloodOxygenSensor::sensorStartCollect() {
    uint8_t buf[2] = {0, 1};
    writeReg(0x20, buf, 2);
}

void BloodOxygenSensor::sensorEndCollect() {
    uint8_t buf[2] = {0, 2};
    writeReg(0x20, buf, 2);
}
