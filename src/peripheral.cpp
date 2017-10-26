#include "peripheral.h"

// #include "bsp_btn_ble.h"
#include "softdevice_handler.h"

#define NRF_LOG_MODULE_NAME "PER"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"

#define APP_FEATURE_NOT_SUPPORTED       BLE_GATT_STATUS_ATTERR_APP_BEGIN + 2

#define OPCODE_LENGTH 1
#define HANDLE_LENGTH 2

PeripheralService *PeripheralService::getRegisteredInstance() {
    if(Peripheral::instance()->getService()) {
        return Peripheral::instance()->getService();
    }
    return NULL;
}

uint32_t PeripheralService::serviceInit()
{
    uint32_t      err_code;
    ble_uuid_t    ble_uuid;

    err_code = sd_ble_uuid_vs_add(&baseUuid, &uuidType);
    VERIFY_SUCCESS(err_code);

    ble_uuid.type = uuidType;
    ble_uuid.uuid = uuid;

    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &ble_uuid,
                                        &serviceHandle);

    return err_code;
}

void PeripheralService::onBleEvent(ble_evt_t *p_ble_evt)
{
    if(p_ble_evt == NULL)
    {
        return;
    }

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            onConnectEvent();
            connectionHandle = p_ble_evt->evt.gap_evt.conn_handle;
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            onDisconnectEvent();
            connectionHandle = BLE_CONN_HANDLE_INVALID;
            break;

        case BLE_GATTS_EVT_WRITE:
            onWriteEvent(&p_ble_evt->evt.gatts_evt.params.write);
            break;

        default:
            break;
    }
}

Peripheral::Peripheral()  :
    service(NULL),
    maxAttMtuSize(NRF_BLE_GATT_MAX_MTU_SIZE),
    connectionHandle(BLE_CONN_HANDLE_INVALID),
    gatt(),
    deviceName("BLEDEV")
{
}

void Peripheral::init(PeripheralService &service, const char* deviceName)
{
    this->service = &service;
    this->deviceName = deviceName;

    bspInit();

    bleInit();

    gapInit();
    gattInit();

    serviceInit();

    advertisingInit();
    connectionParamsInit();

    ble_advertising_start(BLE_ADV_MODE_FAST);
}

void Peripheral::bspInit()
{
    ret_code_t err_code;
    // bsp_event_t startup_event;

    err_code = bsp_init(BSP_INIT_LED, Peripheral::bspEventHandler);
    APP_ERROR_CHECK(err_code);

    // err_code = bsp_btn_ble_init(NULL, &startup_event);
    // APP_ERROR_CHECK(err_code);
}

void Peripheral::bleInit()
{
    uint32_t err_code;

    nrf_clock_lf_cfg_t clock_lf_cfg = NRF_CLOCK_LFCLKSRC;

    // Initialize SoftDevice.
    SOFTDEVICE_HANDLER_INIT(&clock_lf_cfg, NULL);

    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = softdevice_app_ram_start_get(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Overwrite some of the default configurations for the BLE stack.
    ble_cfg_t ble_cfg;

    // Configure the maximum number of connections.
    memset(&ble_cfg, 0, sizeof(ble_cfg));
    ble_cfg.gap_cfg.role_count_cfg.periph_role_count  = BLE_GAP_ROLE_COUNT_PERIPH_DEFAULT;
    ble_cfg.gap_cfg.role_count_cfg.central_role_count = 0;
    ble_cfg.gap_cfg.role_count_cfg.central_sec_count  = 0;
    err_code = sd_ble_cfg_set(BLE_GAP_CFG_ROLE_COUNT, &ble_cfg, ram_start);
    APP_ERROR_CHECK(err_code);

    // Configure the maximum ATT MTU.
    memset(&ble_cfg, 0x00, sizeof(ble_cfg));
    ble_cfg.conn_cfg.conn_cfg_tag                 = Peripheral::connCfgTag;
    ble_cfg.conn_cfg.params.gatt_conn_cfg.att_mtu = NRF_BLE_GATT_MAX_MTU_SIZE;
    err_code = sd_ble_cfg_set(BLE_CONN_CFG_GATT, &ble_cfg, ram_start);
    APP_ERROR_CHECK(err_code);

    // Configure the maximum event length.
    memset(&ble_cfg, 0x00, sizeof(ble_cfg));
    ble_cfg.conn_cfg.conn_cfg_tag                     = Peripheral::connCfgTag;
    ble_cfg.conn_cfg.params.gap_conn_cfg.event_length = 320;
    ble_cfg.conn_cfg.params.gap_conn_cfg.conn_count   = BLE_GAP_CONN_COUNT_DEFAULT;
    err_code = sd_ble_cfg_set(BLE_CONN_CFG_GAP, &ble_cfg, ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = softdevice_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Subscribe for BLE events.
    err_code = softdevice_ble_evt_handler_set(Peripheral::bleEventDispatch);
    APP_ERROR_CHECK(err_code);
}

void Peripheral::gapInit()
{
    uint32_t err_code;
    ble_gap_conn_params_t gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(
        &sec_mode,
        (const uint8_t *)Peripheral::instance()->deviceName,
        strlen(Peripheral::instance()->deviceName));

    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = Peripheral::minConnectionInterval;
    gap_conn_params.max_conn_interval = Peripheral::maxConnectionInterval;
    gap_conn_params.slave_latency     = Peripheral::slaveLatency;
    gap_conn_params.conn_sup_timeout  = Peripheral::superVisionTimeout;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}

void Peripheral::gattInit()
{
    ret_code_t err_code;

    err_code = nrf_ble_gatt_init(&gatt, Peripheral::gattEventHandler);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_ble_gatt_att_mtu_periph_set(&gatt, maxAttMtuSize);
    APP_ERROR_CHECK(err_code);
}

void Peripheral::serviceInit()
{
    service->init(*this);
}

void Peripheral::advertisingInit()
{
    uint32_t err_code;
    ble_advdata_t advdata;
    ble_advdata_t scanrsp;
    ble_adv_modes_config_t options;

    // Build advertising data struct to pass into @ref ble_advertising_init.
    memset(&advdata, 0, sizeof(advdata));
    advdata.name_type          = BLE_ADVDATA_FULL_NAME;
    advdata.include_appearance = false;
    advdata.flags              = BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE;

    adv_uuids[0].uuid = service->getUuid();
    adv_uuids[0].type = BLE_UUID_TYPE_VENDOR_BEGIN;

    memset(&scanrsp, 0, sizeof(scanrsp));
    scanrsp.uuids_complete.uuid_cnt = 1;
    scanrsp.uuids_complete.p_uuids  = adv_uuids;

    memset(&options, 0, sizeof(options));
    options.ble_adv_fast_enabled  = true;
    options.ble_adv_fast_interval = Peripheral::appAdvInterval;
    options.ble_adv_fast_timeout  = Peripheral::appAdvTimeoutInSeconds;

    err_code = ble_advertising_init(&advdata, &scanrsp, &options, Peripheral::onAdvertisingEvent, NULL);
    APP_ERROR_CHECK(err_code);

    ble_advertising_conn_cfg_tag_set(Peripheral::connCfgTag);
}

void Peripheral::connectionParamsInit()
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = Peripheral::firstConnParamsUpdateDelay;
    cp_init.next_conn_params_update_delay  = Peripheral::nextConnParamsUpdateDelay;
    cp_init.max_conn_params_update_count   = Peripheral::maxConnParamsUpdateCount;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = Peripheral::onConnectionParamsEvent;
    cp_init.error_handler                  = Peripheral::connectionParamsErrorHandler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}

void Peripheral::bspEventHandler(bsp_event_t event)
{
    uint32_t err_code;

    switch(event)
    {
        case BSP_EVENT_SLEEP:
            Peripheral::instance()->sleepModeEnter();
            break;

        case BSP_EVENT_DISCONNECT:
            err_code = sd_ble_gap_disconnect(Peripheral::instance()->connectionHandle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            if (err_code != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(err_code);
            }
            break;

        case BSP_EVENT_WHITELIST_OFF:
            if (Peripheral::instance()->connectionHandle == BLE_CONN_HANDLE_INVALID)
            {
                err_code = ble_advertising_restart_without_whitelist();
                if (err_code != NRF_ERROR_INVALID_STATE)
                {
                    APP_ERROR_CHECK(err_code);
                }
            }
            break;

        default:
            break;
    }
}

void Peripheral::bleEventDispatch(ble_evt_t *p_ble_evt)
{
    ble_conn_params_on_ble_evt(p_ble_evt);
    nrf_ble_gatt_on_ble_evt(&Peripheral::instance()->gatt, p_ble_evt);

    Peripheral::instance()->service->onBleEvent(p_ble_evt);
    Peripheral::instance()->onBleEvent(p_ble_evt);

    ble_advertising_on_ble_evt(p_ble_evt);
    // bsp_btn_ble_on_ble_evt(p_ble_evt);
}

void Peripheral::onBleEvent(ble_evt_t * p_ble_evt)
{
    uint32_t err_code;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
            APP_ERROR_CHECK(err_code);
            Peripheral::instance()->connectionHandle = p_ble_evt->evt.gap_evt.conn_handle;
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            err_code = bsp_indication_set(BSP_INDICATE_IDLE);
            APP_ERROR_CHECK(err_code);
            Peripheral::instance()->connectionHandle = BLE_CONN_HANDLE_INVALID;
            break;

        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            // Pairing not supported
            err_code = sd_ble_gap_sec_params_reply(Peripheral::instance()->connectionHandle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
            APP_ERROR_CHECK(err_code);
            break;

         case BLE_GAP_EVT_DATA_LENGTH_UPDATE_REQUEST:
        {
            ble_gap_data_length_params_t dl_params;

            // Clearing the struct will effectivly set members to @ref BLE_GAP_DATA_LENGTH_AUTO
            memset(&dl_params, 0, sizeof(ble_gap_data_length_params_t));
            err_code = sd_ble_gap_data_length_update(p_ble_evt->evt.gap_evt.conn_handle, &dl_params, NULL);
            APP_ERROR_CHECK(err_code);
        } break;

        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            // No system attributes have been stored.
            err_code = sd_ble_gatts_sys_attr_set(Peripheral::instance()->connectionHandle, NULL, 0, 0);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_EVT_USER_MEM_REQUEST:
            err_code = sd_ble_user_mem_reply(p_ble_evt->evt.gattc_evt.conn_handle, NULL);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
        {
            ble_gatts_evt_rw_authorize_request_t  req;
            ble_gatts_rw_authorize_reply_params_t auth_reply;

            req = p_ble_evt->evt.gatts_evt.params.authorize_request;

            if (req.type != BLE_GATTS_AUTHORIZE_TYPE_INVALID)
            {
                if ((req.request.write.op == BLE_GATTS_OP_PREP_WRITE_REQ)     ||
                    (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_NOW) ||
                    (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_CANCEL))
                {
                    if (req.type == BLE_GATTS_AUTHORIZE_TYPE_WRITE)
                    {
                        auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
                    }
                    else
                    {
                        auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_READ;
                    }
                    auth_reply.params.write.gatt_status = APP_FEATURE_NOT_SUPPORTED;
                    err_code = sd_ble_gatts_rw_authorize_reply(p_ble_evt->evt.gatts_evt.conn_handle,
                                                               &auth_reply);
                    APP_ERROR_CHECK(err_code);
                }
            }
        } break;

        default:
            // No implementation needed.
            break;
    }
}

void Peripheral::gattEventHandler(nrf_ble_gatt_t *p_gatt, const nrf_ble_gatt_evt_t *p_evt)
{
    if((Peripheral::instance()->connectionHandle == p_evt->conn_handle) &&
      (p_evt->evt_id == NRF_BLE_GATT_EVT_ATT_MTU_UPDATED)) {

        Peripheral::instance()->maxAttMtuSize = p_evt->params.att_mtu_effective - OPCODE_LENGTH - HANDLE_LENGTH;
        NRF_LOG_INFO("Max ATT MTU size set to 0x%02X (%0d)\r\n", Peripheral::instance()->maxAttMtuSize, Peripheral::instance()->maxAttMtuSize);
    }
}

void Peripheral::onConnectionParamsEvent(ble_conn_params_evt_t * p_evt)
{
    uint32_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(Peripheral::instance()->connectionHandle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}

void Peripheral::connectionParamsErrorHandler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

void Peripheral::onAdvertisingEvent(ble_adv_evt_t ble_adv_evt)
{
    uint32_t err_code;

    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
            APP_ERROR_CHECK(err_code);
            break;
        case BLE_ADV_EVT_IDLE:
            Peripheral::instance()->sleepModeEnter();
            break;
        default:
            break;
    }
}

void Peripheral::sleepModeEnter(void)
{
    uint32_t err_code = bsp_indication_set(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);

#if 0
    // Prepare wakeup buttons.
    err_code = bsp_btn_ble_sleep_mode_prepare();
    APP_ERROR_CHECK(err_code);
#endif
    // Go to system-off mode (this function will not return; wakeup will cause a reset).
    err_code = sd_power_system_off();
    APP_ERROR_CHECK(err_code);
}

void Peripheral::powerManage()
{
    ret_code_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}

uint32_t Char::attach(PeripheralService &service)
{
    this->service = &service;

    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);

    cccd_md.vloc = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read          = read;
    char_md.char_props.write         = write;
    char_md.char_props.write_wo_resp = writeWoResp;
    char_md.char_props.notify        = notify;
    char_md.p_char_user_desc         = NULL;
    char_md.p_char_pf                = NULL;
    char_md.p_user_desc_md           = NULL;
    char_md.p_cccd_md                = &cccd_md;
    char_md.p_sccd_md                = NULL;

    ble_uuid.type = service.getUuidType();
    ble_uuid.uuid = uuid;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    attr_md.vloc    = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen    = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = 0;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = len;

    return sd_ble_gatts_characteristic_add(service.getServiceHandle(),
                                           &char_md,
                                           &attr_char_value,
                                           &handles);
};

uint32_t Char::setValue(uint8_t *value, uint16_t len)
{
    uint32_t err_code;

    if(notify && service != NULL && service->getConnectionHandle() != BLE_CONN_HANDLE_INVALID)
    {
        ble_gatts_hvx_params_t hvx_params;

        memset(&hvx_params, 0, sizeof(hvx_params));

        hvx_params.handle = handles.value_handle;
        hvx_params.p_data = value;
        hvx_params.p_len  = &len;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;

        err_code = sd_ble_gatts_hvx(service->getConnectionHandle(), &hvx_params);
    }
    else {
        ble_gatts_value_t gatts_value;

        memset(&gatts_value, 0, sizeof(gatts_value));

        gatts_value.len     = len;
        gatts_value.offset  = 0;
        gatts_value.p_value = value;

        err_code = sd_ble_gatts_value_set(BLE_CONN_HANDLE_INVALID, handles.value_handle, &gatts_value);
    }

    return err_code;
}

uint32_t Char::getValue(uint8_t *value, uint16_t *len)
{
    uint32_t err_code;

    ble_gatts_value_t gatts_value;

    err_code = sd_ble_gatts_value_get(service->getConnectionHandle(), handles.value_handle, &gatts_value);
    VERIFY_SUCCESS(err_code);

    if(gatts_value.len > *len) {
        err_code = NRF_ERROR_INVALID_PARAM;
    }
    else {
        memcpy(value, gatts_value.p_value, gatts_value.len);
        *len = gatts_value.len;
    }

    return err_code;
}

Peripheral Peripheral::_instance;
