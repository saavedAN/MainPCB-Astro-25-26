#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <stdint.h>
#include <iostream>

// I2C defines
#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9
#define I2C_ADDR 0x28

// Registers (gotta have these)
#define BNO055_CHIP_ID_ADDR     0x00
#define BNO055_PAGE_ID_ADDR     0x07
#define BNO055_PWR_MODE_ADDR    0x3E
#define BNO055_OPR_MODE_ADDR    0x3D
#define BNO055_SYS_TRIGGER_ADDR 0x3F
#define BNO055_ID               0xA0

// Modes
#define OP_MODE_CONFIG          0x00
#define OP_MODE_NDOF            0x0C // The both imu and accel
#define PWR_MODE_NORMAL         0x00



// --- Your Helpers ---
void writeToIMU(uint8_t reg, uint8_t data) {
    uint8_t buf[2] = {reg, data};
    i2c_write_blocking(I2C_PORT, I2C_ADDR, buf, 2, false);
}

void read(uint8_t reg, uint8_t* buffer, size_t length) {
    i2c_write_blocking(I2C_PORT, I2C_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, I2C_ADDR, buffer, length, false);
}

// quick wrapper to read just one byte cause we do it a lot
uint8_t read8(uint8_t reg) {
    uint8_t val;
    read(reg, &val, 1);
    return val;
}

// --- The Meat ---

bool configureIMU() {
    // 1. Check if the chip is even awake
    uint8_t id = read8(BNO055_CHIP_ID_ADDR);
    if (id != BNO055_ID) {
        sleep_ms(1000); // hold on, maybe it's booting
        id = read8(BNO055_CHIP_ID_ADDR);
        if (id != BNO055_ID) {
            printf("BNO055 dead or missing. ID: 0x%02X\n", id);
            return false; // bail
        }
    }

    // 2. Switch to config mode so we can reset
    writeToIMU(BNO055_OPR_MODE_ADDR, OP_MODE_CONFIG);
    sleep_ms(30); // requires 19ms to switch modes

    // 3. Reset the thing
    writeToIMU(BNO055_SYS_TRIGGER_ADDR, 0x20); // 0x20 = reset bit
    sleep_ms(30);

    // 4. Wait for it to come back to life
    while (read8(BNO055_CHIP_ID_ADDR) != BNO055_ID) {
        sleep_ms(10);
    }
    sleep_ms(50); // extra safety buffer

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

uint16_t[12] readSensorData() {
    uint8_t data[24];

    i2c_read_blocking(I2C_PORT, data, 0x08, 24, true);  // 0x08 is start of sensor data ACC_X_LSB

    // each sensor data is 16 bit, accel is first , mag 2nd, gyro 3rd, ori 4th. stored in imus below
    uint16_t imus[12];

    for(int i = 0; i < imus.length(); i++) {
        imus[i] = (int16_t)(data[2*i+1] << 8 | data[2*i]);
    }
    
    return imus;
}

int main()
{
    stdio_init_all();
    sleep_ms(2000); // give the serial monitor a sec to catch up

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    
    // Gotta actually set the pins to I2C mode or nothing happens
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    printf("Booting IMU...\n");
    sleep_ms(1000);
    if (!configureIMU()) {
        while(1) {
            printf("IMU Failed. Stuck loop.\n");
            sleep_ms(1000);
        }
    }

    printf("IMU Ready.\n");

    while(true) {
        // do your sensor stuff
        sleep_ms(100);

    }
    uint16_t imus[12] = readSensorData();  // godspeed little 

    //printing out the data from imus
    std::cout << "Acceleration X Y Z: " << imus[0] << " "  << imus[1] << " " << imus[2] << " \n";
    std::cout << "Magnytometer X Y Z: " << imus[3] << " " << imus[4] << " " << imus[5] << " \n";
    std::cout << "Gyroscope: X Y Z: " << imus[6] << " " << imus[7] << " " << imus[8] << " \n";
    std::cout << "Orientation: X Y Z: " << imus[9] << " " << imus[10] << " " << imus[11] << " \n";

}