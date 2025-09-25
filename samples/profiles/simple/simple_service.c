/*
 * Copyright (c) 2006-2025 LGT Development Team
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-03-01     Evlers       first implementation
 */

#define DBG_TAG             "simple.profile"
#define DBG_LVL             DBG_INFO

#include "rtthread.h"
#include "rtdbg.h"

#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "simple_service.h"

static uint16_t simple_notify_handle;
static simple_write_cb_t simple_write_cb = RT_NULL;

static int gatt_svr_chr_access (uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);

static const struct ble_gatt_svc_def gatt_svr_svcs[] =
{
    {
        /* Service: simple */
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(GATT_SIMPLE_SERVICE_UUID),
        .characteristics = (struct ble_gatt_chr_def[])
        {
            {
                /* Characteristic: simple notify */
                .uuid = BLE_UUID16_DECLARE(GATT_SIMPLE_NOTIFY_UUID),
                .access_cb = gatt_svr_chr_access,
                .val_handle = &simple_notify_handle,
                .flags = BLE_GATT_CHR_F_NOTIFY,
            },
            {
                /* Characteristic: simple write */
                .uuid = BLE_UUID16_DECLARE(GATT_SIMPLE_WRITE_UUID),
                .access_cb = gatt_svr_chr_access,
                .flags = BLE_GATT_CHR_F_WRITE,
            },
            {
                0, /* No more characteristics in this service */
            },
        }
    },

    {
        0, /* No more services */
    },
};

static int gatt_svr_chr_access (uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    uint16_t uuid;

    uuid = ble_uuid_u16(ctxt->chr->uuid);

    if (uuid == GATT_SIMPLE_WRITE_UUID)
    {
        if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR)
        {
            if (simple_write_cb) {
                return simple_write_cb(ctxt->om->om_data, ctxt->om->om_len);
            }
        }
    }

    LOG_E("unknown access uuid: %04x", uuid);
    return BLE_ATT_ERR_UNLIKELY;
}

/**
 * @brief Sends a "free-form" characteristic notification. 
 * 
 * @param conn_handle The connection over which to execute the procedure.
 * @param data The data to send.
 * @param len The length of the data to send.
 * @return int 0 on success; nonzero on failure.
 */
int simple_service_notify (uint16_t conn_handle, void *data, uint16_t len)
{
    struct os_mbuf *om;
    om = ble_hs_mbuf_from_flat(data, len);
    return ble_gatts_notify_custom(conn_handle, simple_notify_handle, om);
}

/**
 * @brief register simple custom service
 * 
 * @param cb simple write callback
 * @return int 0 on success; nonzero on failure.
 */
int simple_service_register (simple_write_cb_t cb)
{
    int rc;

    simple_write_cb = cb;

    rc = ble_gatts_count_cfg(gatt_svr_svcs);
    if (rc != 0) {
        return rc;
    }

    rc = ble_gatts_add_svcs(gatt_svr_svcs);
    if (rc != 0) {
        return rc;
    }

    return 0;
}
