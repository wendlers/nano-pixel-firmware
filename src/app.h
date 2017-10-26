#ifndef __APP_H__
#define __APP_H__

#include "ws2812b_service.h"
#include "ws2812b_spi.h"
#include "ws2812b.h"

class App : public Ws2812bServiceEventHandler
{
public:

    enum {
        CONTROL_UPDATE = 0x01,
        CONTROL_BLANK = 0x02,
    };

public:

    App();

    void init();
    void runForever();

    void onConnect();
    void onDisconnect();

    void onSetLedReceived(uint16_t id, uint8_t r, uint8_t g, uint8_t b);
    void onSetLedReceived(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b);
    void onCtrlReceived(uint8_t cmd);

private:

    Ws2812bService service;
    Ws2812bMatrix<Ws2812bSpi<64, Ws2812bSpiInst::spi0>, 8> leds;
};

#endif
