/*
 * Copyright (c) 2006-2025 LGT Development Team
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-03-01     Evlers       first implementation
 */

#define DBG_TAG             "devinfo.profile"
#define DBG_LVL             DBG_INFO

#include "rtthread.h"
#include "rtdbg.h"

#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "devinfo_service.h"

static const char *manuf_name = "Apache Mynewt";
static const char *model_num = "Mynewt Device Info";

static int gatt_svr_chr_access (uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);

static const struct ble_gatt_svc_def gatt_svr_svcs[] =
{
    {
        /* Service: Device Information */
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(GATT_DEVICE_INFO_UUID),
        .characteristics = (struct ble_gatt_chr_def[])
        {
            {
                /* Characteristic: * Manufacturer name */
                .uuid = BLE_UUID16_DECLARE(GATT_MANUFACTURER_NAME_UUID),
                .access_cb = gatt_svr_chr_access,
                .flags = BLE_GATT_CHR_F_READ,
            },
            {
                /* Characteristic: Model number string */
                .uuid = BLE_UUID16_DECLARE(GATT_MODEL_NUMBER_UUID),
                .access_cb = gatt_svr_chr_access,
                .flags = BLE_GATT_CHR_F_READ,
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
    int rc;

    uuid = ble_uuid_u16(ctxt->chr->uuid);

    if (uuid == GATT_MODEL_NUMBER_UUID) {
        rc = os_mbuf_append(ctxt->om, model_num, strlen(model_num));
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }

    if (uuid == GATT_MANUFACTURER_NAME_UUID) {
        rc = os_mbuf_append(ctxt->om, manuf_name, strlen(manuf_name));
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }

    assert(0);
    return BLE_ATT_ERR_UNLIKELY;
}

/**
 * @brief set manufacturer name
 * 
 * @param name manufacturer name
 */
void devinfo_service_set_manuf_name (const char *name)
{
    manuf_name = name;
}

/**
 * @brief set model number
 * 
 * @param num model number
 */
void devinfo_service_set_model_num (const char *num)
{
    model_num = num;
}

/**
 * @brief register device information service
 * 
 * @return int 0 on success; nonzero on failure.
 */
int devinfo_service_register (void)
{
    int rc;

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
