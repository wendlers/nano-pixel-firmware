#include "app.h"

#define NRF_LOG_MODULE_NAME "APP"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"

App::App() :
    service(*this)
{
}

void App::init()
{
    Peripheral::instance()->init(service, "WS2812B");
}

void App::runForever()
{
    for (;;)
    {
        Peripheral::instance()->powerManage();
    }
}

void App::onConnect()
{
    NRF_LOG_INFO("onConnect\n\r");
}

void App::onDisconnect()
{
    NRF_LOG_INFO("onDisconnect\n\r");
}

void App::onSetLedReceived(uint16_t id, uint8_t r, uint8_t g, uint8_t b)
{
    NRF_LOG_INFO("onSetLedReceived: %d, %d, %d, %d\n\r", id, r, g, b);

    leds.set(id, r, g, b);
}

void App::onSetLedReceived(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b)
{
    NRF_LOG_INFO("onSetLedReceived: %d, %d, %d, %d, %d\n\r", x, y, r, g, b);

    leds.set(x, y, r, g, b);
}

void App::onCtrlReceived(uint8_t cmd)
{
    NRF_LOG_INFO("onCtrlReceived: %d\n\r", cmd);

    switch(cmd) {
        case CONTROL_UPDATE:
            leds.update();
            break;
        case CONTROL_BLANK:
            leds.blank();
            break;
        default:
            NRF_LOG_WARNING("Unknown control command received: %d\n\r", cmd);
    }
}
