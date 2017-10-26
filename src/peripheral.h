#ifndef __PERIPHERAL_H__
#define __PERIPHERAL_H__

#include "bsp.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "nrf_ble_gatt.h"
#include "app_timer.h"

#include "singleton.h"

// #define BLE_FAST

class Peripheral;

class PeripheralService
{
friend class Peripheral;

public:

    PeripheralService(ble_uuid128_t baseUuid, uint16_t uuid) :
        peripheral(NULL),
        serviceHandle(0),
        connectionHandle(BLE_CONN_HANDLE_INVALID),
        baseUuid(baseUuid),
        uuid(uuid),
        uuidType(0)
    {};

    void init(Peripheral &peripheral)
    {
        this->peripheral = &peripheral;

        // TODO think about handling returned errors
        if(serviceInit() == NRF_SUCCESS) {
            charInit();
        }
    };

    static PeripheralService *getRegisteredInstance();

    virtual uint16_t getUuid()
    {
        return uuid;
    };

    virtual uint8_t getUuidType()
    {
        return uuidType;
    };

    virtual uint16_t getServiceHandle()
    {
        return serviceHandle;
    };

    virtual uint16_t getConnectionHandle()
    {
        return connectionHandle;
    };

    virtual bool isConnected()
    {
        return connectionHandle != BLE_CONN_HANDLE_INVALID;
    };

private:

    virtual uint32_t serviceInit();
    virtual uint32_t charInit() { return NRF_SUCCESS; };

    virtual void onBleEvent(ble_evt_t *p_ble_evt);

    virtual void onConnectEvent() {};
    virtual void onDisconnectEvent() {};
    virtual void onWriteEvent(ble_gatts_evt_write_t *p_evt_write) {};

private:

    Peripheral *peripheral;

protected:

    uint16_t serviceHandle;
    uint16_t connectionHandle;

    ble_uuid128_t baseUuid;
    uint16_t      uuid;
    uint8_t       uuidType;
};

class Peripheral
{
    DEF_SINGLETON(Peripheral);

public:

    void init(PeripheralService &service, const char* deviceName);

    uint16_t getMaxAttMtuSize() {
        return maxAttMtuSize;
    };

    PeripheralService *getService() {
        return service;
    }

    void powerManage();

private:

    void bspInit();
    void bleInit();
    void gapInit();
    void gattInit();
    void serviceInit();
    void advertisingInit();
    void connectionParamsInit();

    void onBleEvent(ble_evt_t *p_ble_evt);

    static void gattEventHandler(nrf_ble_gatt_t *p_gatt, const nrf_ble_gatt_evt_t *p_evt);
    static void bspEventHandler(bsp_event_t event);
    static void onConnectionParamsEvent(ble_conn_params_evt_t * p_evt);
    static void connectionParamsErrorHandler(uint32_t nrf_error);
    static void onAdvertisingEvent(ble_adv_evt_t ble_adv_evt);

    static void bleEventDispatch(ble_evt_t *p_ble_evt);

    void sleepModeEnter();

public:

    static const uint8_t connCfgTag = 1;

#ifdef BLE_FAST
    static const uint16_t minConnectionInterval = MSEC_TO_UNITS(16, UNIT_1_25_MS);
    static const uint16_t maxConnectionInterval = MSEC_TO_UNITS(32, UNIT_1_25_MS);
    static const uint16_t slaveLatency = 0;
#else
    static const uint16_t minConnectionInterval = MSEC_TO_UNITS(32, UNIT_1_25_MS);
    static const uint16_t maxConnectionInterval = MSEC_TO_UNITS(64, UNIT_1_25_MS);
    static const uint16_t slaveLatency = 3;
#endif

    static const uint16_t superVisionTimeout = MSEC_TO_UNITS(4000, UNIT_10_MS);

    static const uint32_t firstConnParamsUpdateDelay = APP_TIMER_TICKS(5000);
    static const uint32_t nextConnParamsUpdateDelay = APP_TIMER_TICKS(30000);
    static const uint8_t  maxConnParamsUpdateCount = 3;

    static const uint32_t appAdvInterval = 64;
    static const uint32_t appAdvTimeoutInSeconds = 180;

    static const uint32_t maxAdvUuids = 1;

private:

    PeripheralService *service;
    uint16_t maxAttMtuSize;

    uint16_t connectionHandle;
    nrf_ble_gatt_t gatt;

    const char *deviceName;

    ble_uuid_t adv_uuids[maxAdvUuids];
};

class Char
{
public:

    Char(uint16_t uuid, uint16_t len,
        bool read=false,
        bool write=false,
        bool writeWoResp=false,
        bool notify=false) :
            service(NULL),
            uuid(uuid),
            len(len),
            read(read),
            write(write),
            writeWoResp(writeWoResp),
            notify(notify),
            handles() {};

    uint16_t getUuid() {
        return uuid;
    };

    uint16_t getLen() {
        return len;
    };

    ble_gatts_char_handles_t &getHandles() {
        return handles;
    };

    virtual uint32_t attach(PeripheralService &service);

    uint32_t setValue(uint8_t *value, uint16_t len);
    uint32_t getValue(uint8_t *value, uint16_t *len);

protected:

    PeripheralService *service;

    uint16_t uuid;
    uint16_t len;

    bool read;
    bool write;
    bool writeWoResp;
    bool notify;

    ble_gatts_char_handles_t handles;
};

class ReadChar : public Char
{
public:
    ReadChar(uint16_t uuid, uint16_t len, bool read=true, bool notify=false) :
        Char(uuid, len, read, false, false, notify) {};
};

class WriteChar : public Char
{
public:

    WriteChar(uint16_t uuid, uint16_t len, bool writeWoResp=true) :
        Char(uuid, len, false, true, writeWoResp, false) {};
};

class ReadWriteChar : public Char
{
public:
    ReadWriteChar(uint16_t uuid, uint16_t len, bool writeWoResp=true, bool notify=false) :
        Char(uuid, len, true, true, writeWoResp, notify) {};
};

#endif
