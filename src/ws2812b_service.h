#ifndef __WS2812b_SERVICE_H__
#define __WS2812b_SERVICE_H__

#include "peripheral.h"

class Ws2812bServiceEventHandler
{
public:

    virtual void onConnect() {};
    virtual void onDisconnect() {};

    virtual void onSetLedReceived(uint16_t id, uint8_t r, uint8_t g, uint8_t b) {};
    virtual void onSetLedReceived(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b) {};
    virtual void onCtrlReceived(uint8_t cmd) {};
};

class Ws2812bService : public PeripheralService
{
public:

    static const ble_uuid128_t BASE_UUID;

    static const uint16_t SERVICE_UUID  = 0xAC01;
    static const uint16_t SETLED_UUID   = 0xAC02;
    static const uint16_t CTRL_UUID     = 0xAC03;

public:

    Ws2812bService(Ws2812bServiceEventHandler &eventHandler);

    static Ws2812bService *getRegisteredInstance()
    {
        return reinterpret_cast<Ws2812bService *>(PeripheralService::getRegisteredInstance());
    }

private:

    uint32_t charInit();

    void onConnectEvent();
    void onDisconnectEvent();
    void onWriteEvent(ble_gatts_evt_write_t *p_evt_write);

private:

    Ws2812bServiceEventHandler *eventHandler;

    WriteChar setLedChar;
    WriteChar ctrlChar;
};

#endif
