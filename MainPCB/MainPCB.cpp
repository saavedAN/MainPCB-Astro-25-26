
#include "MainPCB.h"
#include "pico/stdlib.h"
#include <stdint.h>
#include <iostream>
#include <string>
#include "hardware/i2c.h"
#include <stdio.h>

// --- Your Helpers ---
int writeToIMU(uint8_t reg, uint8_t data) {
    uint8_t buf[2] = {reg, data};
    int status = i2c_write_blocking(I2C_PORT, I2C_ADDR, buf, 2, false);
    if(status < 0){
        return BNO55_WRITE_ERROR;
    }
    return status;
}

int readFromIMU(uint8_t reg, uint8_t *buffer, size_t len){
    int status = i2c_write_blocking(I2C_PORT,I2C_ADDR, &reg, 1, true);
    if(status < 0){
        return I2C_WRITE_ERROR;
    }
    status = i2c_read_blocking(I2C_PORT, I2C_ADDR, buffer,len, false);
    if(status < 0){
        return I2C_READ_ERROR;
    }
    return 0;
}

// --- The Meat ---
bool configureIMU() {
    uint8_t buffer;
    int status = readFromIMU(BNO055_CHIP_ID_ADDR, &buffer, 1);
    if(status < 0){
        printf("readFromIMU() has an error");
        return false;
    }
    sleep_ms(1000);
    if(buffer != BNO055_ID){
        printf("BNO055 dead or missing. ID: 0x%02X\n", buffer);
        return false;
    }

    // 2. Switch to config mode so we can reset
    writeToIMU(BNO055_OPR_MODE_ADDR, OP_MODE_CONFIG);
    sleep_ms(30); // requires 19ms to switch modes

    // 3. Reset the thing
    writeToIMU(BNO055_SYS_TRIGGER_ADDR, 0x20); // 0x20 = reset bit
    sleep_ms(30);

    // 5. Set normal power mode
    writeToIMU(BNO055_PWR_MODE_ADDR, PWR_MODE_NORMAL);
    sleep_ms(10);

    // 6. Set page ID to 0 just to be safe
    writeToIMU(BNO055_PAGE_ID_ADDR, 0);

    // 7. Clear the trigger register
    writeToIMU(BNO055_SYS_TRIGGER_ADDR, 0x0);
    sleep_ms(10);

    // 8. Finally set the mode to Fusion (NDOF)
    // This turns on the gyro/accel/mag and fuses the data
    writeToIMU(BNO055_OPR_MODE_ADDR, OP_MODE_NDOF);
    sleep_ms(20);

    return true; // we good
}

int readSensorData(SensorData* imu) {
    uint8_t data[24];

    uint8_t ACC_LSB = 0x08;
    int status = i2c_write_blocking(I2C_PORT, I2C_ADDR, &ACC_LSB, 1, true);
    if(status < 0){
        return I2C_WRITE_ERROR;//i2cwrite has issues
    }
    status = i2c_read_blocking(I2C_PORT, I2C_ADDR, data, 24, false);  // 0x08 is start of sensor data ACC_X_LSB
    if(status < 0){
        return I2C_READ_ERROR;//i2cread has issues
    }    

    // each sensor data is 16 bit, accel is first, mag 2nd, gyro 3rd, ori 4th. stored in imus below
    for(int i = 0; i < 12; i++) {
        imu->imusData[i] = (int16_t)(data[2*i+1] << 8 | data[2*i]);
    }
    return 0;
}

