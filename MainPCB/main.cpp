#include "MainPCB.h"
#include "pico/stdlib.h"
#include <stdint.h>
#include <iostream>
#include <string>
#include "hardware/i2c.h"
#include <stdio.h>

int main()
{
    stdio_init_all();

 
    sleep_ms(10000); // give the serial monitor a sec to catch up

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*10000);
    
    // Gotta actually set the pins to I2C mode or nothing happens
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    printf("Booting IMU...\n");
    sleep_ms(1000);
    if(!configureIMU()) {
        while(true){
            printf(" IMU Failed. Stuck loop.\n");
            sleep_ms(1000);
        }
    }

    printf("IMU Ready.\n");


    SensorData imu;
    //printing out the data from imus
    while(readSensorData(&imu) >=  0) {
        std::cout << "Acceleration X Y Z: " 
        << imu.imusData[0] << " "  << imu.imusData[1] << " " << imu.imusData[2] << " \n";
        std::cout << "Magnytometer X Y Z: " 
        << imu.imusData[3] << " " << imu.imusData[4] << " " << imu.imusData[5] << " \n";
        std::cout << "Gyroscope: X Y Z: " 
        << imu.imusData[6] << " " << imu.imusData[7] << " " << imu.imusData[8] << " \n";
        std::cout << "Orientation: X Y Z: " 
        << imu.imusData[9] << " " << imu.imusData[10] << " " << imu.imusData[11] << " \n";

        sleep_ms(500);
    }
    printf("",readSensorData(&imu));
    return 0;
}