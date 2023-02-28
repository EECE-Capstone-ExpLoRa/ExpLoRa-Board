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
    printf("WTF0");

    // I2C Setup
    I2C i2c(I2C_SDA, I2C_SCL);

    printf("WTF01");

    // ALTIMITER Setup
    const int addrRdAlt = 0xED;
    const int addrWrAlt = 0xEC;
    char rawcal[14];
    unsigned short cal[6];
    
    char cmd[1] = {0x1E};

    i2c.write(addrWrAlt, cmd, 1);
    ThisThread::sleep_for(50);

    printf("WTF1");

    char cmds[7] = {0xA2, 0xA4, 0xA6, 0xA8, 0xAA, 0xAD};
    cmd[0] = 0xA0;
    char precal[2];
    for (int i = 0; i < 6; i++) {
        cmd[0] = cmds[i];
        i2c.write(addrWrAlt, cmd, 1);
        i2c.read(addrRdAlt, precal, 2);
        rawcal[i * 2] = precal[0];
        rawcal[(i * 2) + 1] = precal[1];
    }
    printf("WTF2");

    for (int i = 0; i < 6; i++) {
        cal[i] = (((unsigned short)rawcal[i * 2]) << 8) | (unsigned short)rawcal[(i * 2) + 1];
        printf("C%d = %d\n", i + 1, cal[i]);
    }

    char cmdPres[1] = {0x46};
    char cmdTemp[1] = {0x56};
    char cmdRdADC[1] = {0x0};
    char rawdata[3];
    unsigned int dataAlt[2];

    // IMU Setup
    const int addrRdIMU = 0xD5;
    const int addrWrIMU = 0xD4;
    const int addrConfig = 0x10;
    const char config[11] = {addrConfig, 0b01011100,0b01010100,0x04,0x0,0x0,0x0,0x0,0x0,0xE0,0x0}; // Config address, followed by 10 config registers
    const float accelScale = 0.00024415;
    const float gyroScale = 0.01525925;

    i2c.write(addrWrIMU, config, 11);

    char addr[1] = {0x22};
    char dataIMU[12];
    short gyroAccelData[6];

    // AIR SENSOR Setup


    while (true) {
        // ALTIMITER
        i2c.write(addrWrAlt, cmdPres, 1);
        ThisThread::sleep_for(15);
        i2c.write(addrWrAlt, cmdRdADC, 1);
        i2c.read(addrRdAlt, rawdata, 3);

        dataAlt[0] = (((unsigned int)rawdata[0]) << 16) | (((unsigned int)rawdata[0]) << 8)  | (unsigned int)rawdata[2];

        i2c.write(addrWrAlt, cmdTemp, 1);
        ThisThread::sleep_for(15);
        i2c.write(addrWrAlt, cmdRdADC, 1);
        i2c.read(addrRdAlt, rawdata, 3);
 
        dataAlt[1] = (((unsigned int)rawdata[0]) << 16) | (((unsigned int)rawdata[0]) << 8)  | (unsigned int)rawdata[2];

        int dT = dataAlt[1] - (cal[4] * 256);
        int TEMP = 2000 + dT * cal[5] / 8388608;

        long OFF2 = 0;
        long SENS2 = 0;
        int T2 = 0;
        if (TEMP < 2000) {
            T2 = 3 * dT * dT / 8589934592;
            OFF2 = 61 * (TEMP - 2000) * (TEMP - 2000) / 16;
            SENS2 = 29 * (TEMP - 2000) * (TEMP - 2000) / 16;
            if (TEMP < -1500) {
                OFF2 = OFF2 + 17 * (TEMP + 1500) * (TEMP + 1500);
                SENS2 = SENS2 + 9 * (TEMP + 1500) * (TEMP + 1500);
            }
        } else {
            T2 = 5 * dT * dT / 274877906944;
        }
        TEMP = TEMP - T2;

        long OFF = cal[1] * 131072 + (cal[3] * dT) / 64;
        OFF = OFF - OFF2;
        long SENS = cal[0] * 65536 + (cal[2] * dT) / 128;
        SENS = SENS - SENS2;
        int P = (dataAlt[0] * SENS / 2097152 - OFF) / 32768;

        printf("Temperature = %2.2f degC\n", ((float)TEMP / 100));
        printf("Pressure = %3.2f mbar\n", ((float)P / 100));


        // IMU
        i2c.write(addrWrIMU, addr, 1);
        i2c.read(addrRdIMU, dataIMU, 12);
 
        for (int i = 0; i < 6; i++) {
            gyroAccelData[i] = (((short)dataIMU[i * 2 + 1]) << 8) | (short)dataIMU[i * 2];
        }

        printf("Gyro X = %+3.1f dps\n", (float)gyroAccelData[0] * gyroScale);
        printf("Gyro Y = %+3.1f dps\n", (float)gyroAccelData[1] * gyroScale);
        printf("Gyro Z = %+3.1f dps\n", (float)gyroAccelData[2] * gyroScale);
        printf("Accel X = %+1.2f g\n", (float)gyroAccelData[3] * accelScale);
        printf("Accel Y = %+1.2f g\n", (float)gyroAccelData[4] * accelScale);
        printf("Accel Z = %+1.2f g\n", (float)gyroAccelData[5] * accelScale);


        // AIR SENSOR


        ThisThread::sleep_for(250);
    }
}
