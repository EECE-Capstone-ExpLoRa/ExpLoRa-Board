#ifndef __RADIO_EVENT_H__
#define __RADIO_EVENT_H__

#include "mDotEvent.h"

class RadioEvent : public mDotEvent
{

public:
    RadioEvent() {}

    virtual ~RadioEvent() {}

    /*!
     * MAC layer event callback prototype.
     *
     * \param [IN] flags Bit field indicating the MAC events occurred
     * \param [IN] info  Details about MAC events occurred
     */
    virtual void MacEvent(LoRaMacEventFlags* flags, LoRaMacEventInfo* info) {
        std::string msg = "OK";
        switch (info->Status) {
            case LORAMAC_EVENT_INFO_STATUS_ERROR:
                msg = "ERROR";
                break;
            case LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT:
                msg = "TX_TIMEOUT";
                break;
            case LORAMAC_EVENT_INFO_STATUS_RX_TIMEOUT:
                msg = "RX_TIMEOUT";
                break;
            case LORAMAC_EVENT_INFO_STATUS_RX_ERROR:
                msg = "RX_ERROR";
                break;
            case LORAMAC_EVENT_INFO_STATUS_JOIN_FAIL:
                msg = "JOIN_FAIL";
                break;
            case LORAMAC_EVENT_INFO_STATUS_DOWNLINK_FAIL:
                msg = "DOWNLINK_FAIL";
                break;
            case LORAMAC_EVENT_INFO_STATUS_ADDRESS_FAIL:
                msg = "ADDRESS_FAIL";
                break;
            case LORAMAC_EVENT_INFO_STATUS_MIC_FAIL:
                msg = "MIC_FAIL";
                break;
            default:
                break;
        }
        printf("Event: %s\n", msg.c_str());

        if (flags->Bits.Rx) {
            printf("Rx %d bytes\n", info->RxBufferSize);
        }
    }
};

#endif
