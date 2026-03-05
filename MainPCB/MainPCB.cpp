
#include "MainPCB.h"
#include "pico/stdlib.h"
#include <stdint.h>
#include <iostream>
#include <string>
#include "hardware/i2c.h"
#include <stdio.h>

// --- The Meat ---
bool configureIMU() {
    uint8_t reg = BNO055_CHIP_ID_ADDR;
    uint8_t buffer;
    sleep_ms(1000);
    int status = i2c_write_blocking(I2C_PORT,I2C_ADDR,&reg,1,true);
    printf("check point1");
    status = i2c_read_blocking(I2C_PORT,I2C_ADDR,&buffer,1,false);
    printf("check point2");
    if(buffer != BNO055_ID){
        printf("BNO055 dead or missing. ID: 0x%02X\n", buffer);
        return false;
    }
    //entering config mode
    i2c_write_blocking(I2C_PORT, I2C_ADDR, (uint8_t[]){BNO055_OPR_MODE_ADDR, OP_MODE_CONFIG}, 2, false);
    sleep_ms(50);

    //reset all status bits
    i2c_write_blocking(I2C_PORT, I2C_ADDR, (uint8_t[]){BNO055_SYS_TRIGGER_ADDR,0x20}, 2, false);
    sleep_ms(700); //gotta give it some times to acknowledge or it NACKS(apparently)

    // Configure Power Mode
    i2c_write_blocking(I2C_PORT, I2C_ADDR, (uint8_t[]){BNO055_PWR_MODE_ADDR,PWR_MODE_NORMAL}, 2, false);
    sleep_ms(5);

    //initializing internal oscillator
    //good for higher fusion accuracy
    uint8_t data[2];
    data[0] = 0x3F;
    data[1] = 0x40;
    i2c_write_blocking(I2C_PORT, I2C_ADDR, data, 2, false);
    sleep_ms(5);


    // Set units to m/s^2
    data[0] = 0x3B;
    data[1] = 0b00001000;
    i2c_write_blocking(I2C_PORT, I2C_ADDR, data, 2, false);
    sleep_ms(5);

    //configuring operation mode
    i2c_write_blocking(I2C_PORT, I2C_ADDR,
        (uint8_t[]){BNO055_OPR_MODE_ADDR,OP_MODE_NDOF},2,false);
    sleep_ms(50);
    
    printf("Yippeee!!!");
    return true; // we good
}

int readSensorData(SensorData* imu) {
    uint8_t data[24];
    uint8_t ACC_LSB = 0x08;

    if(i2c_write_blocking(I2C_PORT, I2C_ADDR, &ACC_LSB, 1, true) != 1)
        return WRITE_SENSOR_DATA_ERROR;

    if(i2c_read_blocking(I2C_PORT, I2C_ADDR, data, 24, false) != 24)
        return READ_SENSOR_DATA_ERROR;

    for(int i = 0; i < 12; i++)
        imu->imusData[i] = (int16_t)((data[2*i+1] << 8) | data[2*i]);

    return 1;
}

