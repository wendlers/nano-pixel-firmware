#include "ws2812b_service.h"

#define NRF_LOG_MODULE_NAME "WS2"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"

const ble_uuid128_t Ws2812bService::BASE_UUID = {
    /* https://www.uuidgenerator.net/ */
    {0x76, 0x5e, 0xf0, 0xcc, 0xd5, 0x1b, 0x48, 0x34, 0xbe, 0xbd, 0x5b, 0xaf, 0x2e, 0x41, 0x59, 0x9e}
};

Ws2812bService::Ws2812bService(Ws2812bServiceEventHandler &eventHandler) :
    PeripheralService(BASE_UUID, SERVICE_UUID),
    eventHandler(&eventHandler),
    setLedChar(SETLED_UUID, 2 + 2 + 1 + 1 + 1), /* X-pos, Y-pos, Red, Green, Blue */
    ctrlChar(CTRL_UUID, 1)
{
}

uint32_t Ws2812bService::charInit()
{
    uint32_t err_code;

    err_code = setLedChar.attach(*this);
    VERIFY_SUCCESS(err_code);

    err_code = ctrlChar.attach(*this);
    VERIFY_SUCCESS(err_code);

    return NRF_SUCCESS;
}

void Ws2812bService::onConnectEvent()
{
    eventHandler->onConnect();
}

void Ws2812bService::onDisconnectEvent()
{
    eventHandler->onDisconnect();
}

void Ws2812bService::onWriteEvent(ble_gatts_evt_write_t *p_evt_write)
{
    if(p_evt_write->handle == setLedChar.getHandles().value_handle)
    {
        if(p_evt_write->len == 7) {

            struct setled_t {
                uint16_t x;
                uint16_t y;
                uint8_t r;
                uint8_t g;
                uint8_t b;
            };

            setled_t *l = reinterpret_cast<setled_t *>(p_evt_write->data);
            eventHandler->onSetLedReceived(l->x, l->y, l->r, l->g, l->b);
        }
        else if(p_evt_write->len == 5) {

            struct setled_t {
                uint16_t id;
                uint8_t r;
                uint8_t g;
                uint8_t b;
            };

            setled_t *l = reinterpret_cast<setled_t *>(p_evt_write->data);
            eventHandler->onSetLedReceived(l->id, l->r, l->g, l->b);
        }
        else {
            NRF_LOG_WARNING("Wrong message size (%d) for setLed!\n\r", p_evt_write->len)
        }
    }
    else if(p_evt_write->handle == ctrlChar.getHandles().value_handle)
    {
        if(p_evt_write->len == 1) {
            eventHandler->onCtrlReceived(p_evt_write->data[0]);
        }
        else {
            NRF_LOG_WARNING("Wrong message size (%d) for ctrl!\n\r", p_evt_write->len)
        }
    }
}
