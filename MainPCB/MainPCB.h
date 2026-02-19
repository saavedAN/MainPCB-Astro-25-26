#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <stdint.h>
#include <iostream>

// I2C defines
#define I2C_PORT i2c0
#define I2C_SDA 1
#define I2C_SCL 2
#define I2C_ADDR 0x28

// Registers (gotta have these)
#define BNO055_CHIP_ID_ADDR     0x00
#define BNO055_PAGE_ID_ADDR     0x07
#define BNO055_PWR_MODE_ADDR    0x3E
#define BNO055_OPR_MODE_ADDR    0x3D
#define BNO055_SYS_TRIGGER_ADDR 0x3F
#define BNO055_ID               0xA0

#define BNO55_WRITE_ERROR -1

// Modes
#define OP_MODE_CONFIG          0x00
#define OP_MODE_NDOF            0x0C // The both imu and accel
#define PWR_MODE_NORMAL         0x00

typedef struct {
    uint16_t imusData[12];
}SensorData;

void readSensorData(SensorData* imu);
int writeToIMU(uint8_t reg, uint8_t data);
int readFromIMU(uint8_t reg, uint8_t *buffer, size_t len);
bool configureIMU();