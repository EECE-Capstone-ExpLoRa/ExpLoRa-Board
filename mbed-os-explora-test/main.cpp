/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "dot_util.h"
#include "RadioEvent.h"

#include "mbed.h"
#include "I2C.h"
#include "PinNames.h"
#include "ThisThread.h"

// Blinking rate in milliseconds
#define BLINKING_RATE     500ms


// Lora configuration information
static std::string network_name = "MultiTech";
static std::string network_passphrase = "MultiTech";
static uint8_t network_id[] = { 0x6C, 0x4E, 0xEF, 0x66, 0xF4, 0x79, 0x86, 0xA6 };
static uint8_t network_key[] = { 0x1F, 0x33, 0xA1, 0x70, 0xA5, 0xF1, 0xFD, 0xA0, 0xAB, 0x69, 0x7A, 0xAE, 0x2B, 0x95, 0x91, 0x6B };
static uint8_t frequency_sub_band = 0;
static lora::NetworkType network_type = lora::PUBLIC_LORAWAN;
static uint8_t join_delay = 5;
static uint8_t ack = 0;
static bool adr = false;

mDot* dot = NULL;
lora::ChannelPlan* plan = NULL;

I2C i2c(I2C_SDA, I2C_SCL);


void configureLora();
void configureAccelGyro();
void readAccelGyro(char* buf);


int main()
{
    // // Initialise the digital pin LED1 as an output
    // DigitalOut led1(PA_4);
    // DigitalOut led2(PA_5);
    // DigitalOut led3(PB_0);

    // led1 = false;
    // led2 = false;
    // led3 = false;

    mts::MTSLog::setLogLevel(mts::MTSLog::TRACE_LEVEL);

    RadioEvent radioEvents;
    plan = create_channel_plan();
    dot = mDot::getInstance(plan);
    dot->setEvents(&radioEvents);

    configureLora();
    configureAccelGyro();

    printf("Configuration Complete!\n");

    while (true) {
        std::vector<uint8_t> tx_gyro;
        std::vector<uint8_t> tx_accel;

        // Set Data IDs
        tx_gyro.push_back(0x04); 
        tx_accel.push_back(0x05);

        // join network if not joined
        if (!dot->getNetworkJoinStatus()) {
            join_network();
        }

        // Get data
        char data[12];
        readAccelGyro(data);

        for (int i = 0; i < 6; i++) {
            tx_gyro.push_back(data[i]);
        }
        for (int i = 6; i < 12; i++) {
            tx_accel.push_back(data[i]);
        }

        send_data(tx_gyro);
        send_data(tx_accel);

        ThisThread::sleep_for(3000);
    }
    return 0;
}

void configureLora()
{
    if (!dot->getStandbyFlag() && !dot->getPreserveSession()) {
        printf("mbed-os library version: %d.%d.%d\n", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);

        // start from a well-known state
        dot->resetConfig();
        dot->resetNetworkSession();

        // make sure library logging is turned on
        dot->setLogLevel(mts::MTSLog::INFO_LEVEL);

        // update configuration if necessary
        if (dot->getJoinMode() != mDot::OTA) {
            printf("changing network join mode to OTA\n");
            if (dot->setJoinMode(mDot::OTA) != mDot::MDOT_OK) {
                printf("failed to set network join mode to OTA\n");
            }
        }

        update_ota_config_name_phrase(network_name, network_passphrase, frequency_sub_band, network_type, ack);
        //update_ota_config_id_key(network_id, network_key, frequency_sub_band, network_type, ack);

        //dot->setDefaultFrequencyBand(...);

        // Set Tx Params
        dot->setTxFrequency(lora::BW_500);
        dot->setTxDataRate(mDot::DR4); 
        dot->setTxPower(30);

        // configure network link checks
        // for count = 3 and threshold = 5, the Dot will ask for a link check response every 5 packets and 
        // will consider the connection lost if it fails to receive 3 responses in a row
        update_network_link_check_config(3, 5);

        // disable Adaptive Data Rate
        dot->setAdr(adr);

        // Configure the join delay
        dot->setJoinDelay(join_delay);

        // save changes to configuration
        printf("saving configuration\n");
        if (!dot->saveConfig()) {
            printf("failed to save configuration\n");
        }

        // display configuration
        display_config();
    } else {
        // restore the saved session if the dot woke from deepsleep mode
        // useful to use with deepsleep because session info is otherwise lost when the dot enters deepsleep
        printf("restoring network session from NVM\n");
        dot->restoreNetworkSession();
    }
}

void configureAccelGyro()
{
    const int addrWr = 0xD4;
    const int addrConfig = 0x10;
    const char config[11] = {addrConfig,0b01011100,0b01010100,0x04,0x0,0x0,0x0,0x0,0x0,0xE0,0x0}; // Config address, followed by 10 config registers
    i2c.write(addrWr, config, 11);
}

void readAccelGyro(char* buf) {
    const int addrRd = 0xD5;
    const int addrWr = 0xD4;
    char addr[1] = {0x22};
    char data[12];
    short gyroAccelData[6];

    const float accelScale = 0.00024415;
    const float gyroScale = 0.01525925;

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

    memcpy(buf, data, 12);
}