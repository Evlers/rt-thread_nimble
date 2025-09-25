/*
 * Copyright (c) 2025-2025 LGT Development Team
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-03-01     Evlers       first implementation
 */

#define DBG_TAG             "nimble"
#define DBG_LVL             DBG_INFO

#include "rtthread.h"
#include "rtdevice.h"
#include "rtdbg.h"

#include "host/ble_hs.h"
#include "host/util/util.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "devinfo_service.h"
#include "simple_service.h"

static bool sync_done = false;
static uint8_t own_addr_type;
static bool advertising_is_on = false;
static uint16_t conn_handle = BLE_HS_CONN_HANDLE_NONE;
static char device_name[MYNEWT_VAL(BLE_SVC_GAP_DEVICE_NAME_MAX_LENGTH)] = { "nimble" };

static int ble_gap_event (struct ble_gap_event *event, void *arg);

static void advertise_begen (void)
{
    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields;
    int rc;

    /*
     *  Set the advertisement data included in our advertisements:
     *     o Flags (indicates advertisement type and other general info)
     *     o Advertising tx power
     *     o Device name
     */
    memset(&fields, 0, sizeof(fields));

    /*
     * Advertise two flags:
     *      o Discoverability in forthcoming advertisement (general)
     *      o BLE-only (BR/EDR unsupported)
     */
    fields.flags = BLE_HS_ADV_F_DISC_GEN |
                    BLE_HS_ADV_F_BREDR_UNSUP;

    /*
     * Indicate that the TX power level field should be included; have the
     * stack fill this value automatically.  This is done by assigning the
     * special value BLE_HS_ADV_TX_PWR_LVL_AUTO.
     */
    fields.tx_pwr_lvl_is_present = 1;
    fields.tx_pwr_lvl = 0;

    fields.name = (uint8_t *)device_name;
    fields.name_len = strlen(device_name);
    fields.name_is_complete = 1;

    fields.uuids16 = (ble_uuid16_t[]) {
        BLE_UUID16_INIT(GATT_SIMPLE_SERVICE_UUID),
    };
    fields.num_uuids16 = 1;
    fields.uuids16_is_complete = 1;

    rc = ble_gap_adv_set_fields(&fields);
    if (rc != 0) {
        LOG_E("error setting advertisement data; rc=%d", rc);
        return;
    }

    /* Begin advertising */
    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER,
                           &adv_params, ble_gap_event, NULL);
    if (rc != 0) {
        LOG_E("error enabling advertisement; rc=%d", rc);
        return;
    }
    LOG_I("start advertising, device name: %s", device_name);
    advertising_is_on = true;
}

static int ble_gap_event (struct ble_gap_event *event, void *arg)
{
    switch (event->type)
    {
        case BLE_GAP_EVENT_CONNECT:
            /* A new connection was established or a connection attempt failed */
            LOG_I("connection %s; status=%d, con_handle=%d",
                        event->connect.status == 0 ? "established" : "failed",
                        event->connect.status, event->connect.conn_handle);

            if (event->connect.status != 0) {
                /* Connection failed; resume advertising */
                advertise_begen();
                conn_handle = BLE_HS_CONN_HANDLE_NONE;
            }
            else {
                conn_handle = event->connect.conn_handle;
            }

        break;

        case BLE_GAP_EVENT_DISCONNECT:
            LOG_I("disconnect; reason=%d", event->disconnect.reason);
            conn_handle = BLE_HS_CONN_HANDLE_NONE; /* reset conn_handle */

            /* Connection terminated; resume advertising */
            if (advertising_is_on) {
                advertise_begen();
            }
        break;

        case BLE_GAP_EVENT_ADV_COMPLETE:
            LOG_D("adv complete");
            if (advertising_is_on) {
                advertise_begen();
            }
        break;

        case BLE_GAP_EVENT_SUBSCRIBE:
            LOG_D("subscribe event: attr_handle = %d, cur_notify=%d",
                    event->subscribe.attr_handle, event->subscribe.cur_notify);
        break;

        case BLE_GAP_EVENT_MTU:
            LOG_I("mtu update event; conn_handle=%d mtu=%d",
                        event->mtu.conn_handle,
                        event->mtu.value);
        break;

    }

    return 0;
}

static void gatt_register_cb (struct ble_gatt_register_ctxt *ctxt, void *arg)
{
#if DBG_LVL >= DBG_LOG
    char buf[BLE_UUID_STR_LEN];
#endif

    switch (ctxt->op)
    {
        case BLE_GATT_REGISTER_OP_SVC:
            LOG_D("registered service %s with handle=%d",
                        ble_uuid_to_str(ctxt->svc.svc_def->uuid, buf),
                        ctxt->svc.handle);
        break;

        case BLE_GATT_REGISTER_OP_CHR:
            LOG_D("registering characteristic %s with "
                            "def_handle=%d val_handle=%d",
                        ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf),
                        ctxt->chr.def_handle,
                        ctxt->chr.val_handle);
        break;

        case BLE_GATT_REGISTER_OP_DSC:
            LOG_D("registering descriptor %s with handle=%d",
                        ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buf),
                        ctxt->dsc.handle);
        break;

        default:
            assert(0);
        break;
    }
}

static int simple_write_callback (void *data, uint16_t len)
{
#ifdef RT_USING_ULOG
    ulog_hexdump(DBG_TAG ": simple service write", 16, data, len);
#else
    LOG_HEX(DBG_TAG ": simple service write", 16, data, len);
#endif /* RT_USING_ULOG */
    return 0;
}

static void ble_app_set_addr (void)
{
    ble_addr_t addr;
    int rc;

    /* generate new non-resolvable private address */
    rc = ble_hs_id_gen_rnd(0, &addr);
    assert(rc == 0);

    /* set generated address */
    rc = ble_hs_id_set_rnd(addr.val);
    assert(rc == 0);
}

static void on_sync (void)
{
    /* generate a non-resolvable private address. */
    ble_app_set_addr();

    /* own_addr_type will store type of addres our BSP uses */
    ble_hs_util_ensure_addr(0);
    ble_hs_id_infer_auto(0, &own_addr_type);

    /* begin advertising */
    // advertise_begen();

    sync_done = true;
}

#ifdef RT_USING_WIFI_HOST_DRIVER
static void reset_hci_dev (void)
{
    rt_base_t pin_reg_on = rt_pin_get("PE.11");
    rt_pin_mode(pin_reg_on, PIN_MODE_OUTPUT);
    rt_pin_write(pin_reg_on, PIN_LOW); /* REG_ON = 0 */
    rt_thread_mdelay(2);
    rt_pin_write(pin_reg_on, PIN_HIGH); /* REG_ON = 1 */
}
#endif /* RT_USING_WIFI_HOST_DRIVER */

static void ble_hs_reset (int reason)
{
#ifdef RT_USING_WIFI_HOST_DRIVER
    reset_hci_dev();
#endif /* RT_USING_WIFI_HOST_DRIVER */
    LOG_E("resetting state; reason=%d", reason);
}

int ble_peripheral_init (void)
{
#ifdef RT_USING_WIFI_HOST_DRIVER
    reset_hci_dev();
#endif /* RT_USING_WIFI_HOST_DRIVER */

    /* initialize the NimBLE host configuration */
    ble_hs_cfg.sync_cb = on_sync;
    ble_hs_cfg.reset_cb = ble_hs_reset;
    ble_hs_cfg.gatts_register_cb = gatt_register_cb,

    /* register gap and gatt service */
    ble_svc_gap_init();
    ble_svc_gatt_init();

    /* register device info service */
    devinfo_service_register();
    devinfo_service_set_manuf_name("Nimble");
    devinfo_service_set_model_num("12345678");

    /* register simple service */
    simple_service_register(simple_write_callback);

    /* set the default device name */
    ble_svc_gap_device_name_set(device_name);

    /* startup bluetooth host stack*/
    ble_hs_thread_startup();

    return 0;
}

#if defined(RT_USING_WIFI_HOST_DRIVER) || defined(PKG_USING_WIFI_HOST_DRIVER) || defined(RT_USING_ESP_HOSTED) || defined(PKG_USING_ESP_HOSTED)
/* Applicable to esp-hosted and wifi-host-driver
 * This function is called when the host driver is ready.
 * It initializes the NimBLE host stack and registers the necessary services.
 */

#if defined(RT_USING_WIFI_HOST_DRIVER) || defined(PKG_USING_WIFI_HOST_DRIVER)
const char *interface = "hci uart";
/* when the firmware is ready, whd calls this function */
void whd_bt_startup (void)
#endif /* defined(RT_USING_WIFI_HOST_DRIVER) || defined(PKG_USING_WIFI_HOST_DRIVER) */
#if defined(RT_USING_ESP_HOSTED) || defined(PKG_USING_ESP_HOSTED)
/* when the vhci interface is ready, esp-hosted calls this function */
const char *interface = "esp-hosted";
void esp_hosted_bt_startup (void)
#endif /* RT_USING_ESP_HOSTED */
{
    LOG_I("nimble attach to %s", interface);
    ble_peripheral_init();
}

#else
/* Use the command line interface to initialize NimBLE
 * This is used when the NimBLE stack is not used as a host driver.
 * It allows the user to control Bluetooth operations via command line.
 */

#define USING_CMDLINE_INIT
/* This variable is used to ensure that the peripheral initialization is done only once */
static bool ble_peripheral_init_done = false;

#endif /* defined(RT_USING_WIFI_HOST_DRIVER) || defined(PKG_USING_WIFI_HOST_DRIVER) || defined(RT_USING_ESP_HOSTED) || defined(PKG_USING_ESP_HOSTED) */

static int wait_for_sync (void)
{
    uint32_t timeout = 0;

    /* wait for the stack to be ready */
    while (!sync_done)
    {
        rt_thread_mdelay(1);
        timeout++;
        if (timeout > 3000)
        {
            return -1;
        }
    }

    return 0;
}

/**
 * @brief NimBLE command line interface
 *
 * This function provides a command line interface for controlling NimBLE
 * operations such as starting/stopping advertising, disconnecting, and sending
 * notifications.
 *
 * @param argc Number of arguments
 * @param argv Array of argument strings
 *
 * @return 0 on success, -1 on error
 */
static int nimble_cmd (int argc, char **argv)
{
    if (argc < 2)
    {
        __help:
        rt_kprintf("Usage: ble <command>\n");
        rt_kprintf("This command can use the following:\n");
        rt_kprintf("\tadv\tAdvertising control, <start|stop>\n");
        rt_kprintf("\t\t    start\tStart advertising, optional [name] argument\n");
        rt_kprintf("\t\t    stop\tStop advertising\n");
        rt_kprintf("\tdisc\tTermination connection\n");
        rt_kprintf("\tnotify\tSend notify, example: ble notify <str data>\n");
        return 0;
    }

#ifdef USING_CMDLINE_INIT
    if (!ble_peripheral_init_done)
    {
        uint32_t timeout;
        ble_peripheral_init();
        ble_peripheral_init_done = true;
    }
#endif

    if (wait_for_sync() < 0)
    {
#ifdef USING_CMDLINE_INIT
        ble_peripheral_init_done = true;
#endif /* USING_CMDLINE_INIT */
        rt_kprintf("nimble not ready, please wait...\n");
        return -1;
    }

    if (rt_strcmp(argv[1], "adv") == 0)
    {
        if (argc < 3)
        {
            __adv_help:
            rt_kprintf("Usage: ble adv <start|stop>\n");
            rt_kprintf("\tstart\tstart advertising, optional [name] argument\n");
            rt_kprintf("\tstop\tstop advertising\n");
            return 0;
        }

        if (rt_strcmp(argv[2], "start") == 0)
        {
            if (argc >= 4)
            {
                if (rt_strlen(argv[3]) > MYNEWT_VAL(BLE_SVC_GAP_DEVICE_NAME_MAX_LENGTH))
                {
                    rt_kprintf("Device name is too long!\n");
                    return 0;
                }
                ble_svc_gap_device_name_set(argv[3]);
                rt_memset(device_name, 0, sizeof(device_name));
                rt_memcpy(device_name, argv[3], rt_strlen(argv[3]));
            }

            advertise_begen();
        }
        else if (rt_strcmp(argv[2], "stop") == 0)
        {
            if (advertising_is_on)
            ble_gap_adv_stop();
            advertising_is_on = false;
            LOG_I("advertising stopped");
        }
        else
        {
            goto __adv_help;
        }
    }
    else if (rt_strcmp(argv[1], "disc") == 0)
    {
        if (conn_handle == BLE_HS_CONN_HANDLE_NONE)
        {
            rt_kprintf("Bluetooth is not connected!\n");
            return 0;
        }

        ble_gap_terminate(conn_handle, BLE_ERR_REM_USER_CONN_TERM);
    }
    else if (rt_strcmp(argv[1], "notify") == 0)
    {
        if (argc < 3)
        {
            rt_kprintf("Usage: ble notify <str data>\n");
            return 0;
        }

        if (conn_handle == BLE_HS_CONN_HANDLE_NONE)
        {
            rt_kprintf("Bluetooth is not connected!\n");
            return 0;
        }

        simple_service_notify(conn_handle, argv[2], rt_strlen(argv[2]));
    }
    else
    {
        goto __help;
    }

    return 0;
}
MSH_CMD_EXPORT_ALIAS(nimble_cmd, ble, Bluetooth control);
