/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "I2C.h"
#include "PinNames.h"
#include "ThisThread.h"
#include "mbed.h"


// Blinking rate in milliseconds
#define BLINKING_RATE     500ms


int main()
{
    // Initialise the digital pin LED1 as an output
    DigitalOut led1(PA_4);
    DigitalOut led2(PA_5);
    DigitalOut led3(PB_0);

    led1 = false;
    led2 = false;
    led3 = false;

    const int addrRd = 0xD5;
    const int addrWr = 0xD4;
    const int addrConfig = 0x10;
    const char config[11] = {addrConfig, 0b01011100,0b01010100,0x04,0x0,0x0,0x0,0x0,0x0,0xE0,0x0}; // Config address, followed by 10 config registers
    const float accelScale = 0.00024415;
    const float gyroScale = 0.01525925;

    I2C i2c(I2C_SDA, I2C_SCL);

    i2c.write(addrWr, config, 11);

    char addr[1] = {0x22};
    char data[12];
    short gyroAccelData[6];
    while (true) {
        i2c.write(addrWr, addr, 1);
        i2c.read(addrRd, data, 12);
 
        for (int i = 0; i < 6; i++) {
            gyroAccelData[i] = (((short)data[i * 2 + 1]) << 8) | (short)data[i * 2];
        }

        printf("Gyro X = %+3.1f dps\n", (float)gyroAccelData[0] * gyroScale);
        printf("Gyro Y = %+3.1f dps\n", (float)gyroAccelData[1] * gyroScale);
        printf("Gyro Z = %+3.1f dps\n", (float)gyroAccelData[2] * gyroScale);
        printf("Accel X = %+1.2f g\n", (float)gyroAccelData[3] * accelScale);
        printf("Accel Y = %+1.2f g\n", (float)gyroAccelData[4] * accelScale);
        printf("Accel Z = %+1.2f g\n", (float)gyroAccelData[5] * accelScale);

        ThisThread::sleep_for(75);
    }
}
