/*
 * Copyright (c) 2006-2025 RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-01-19     Evlers       the first version
 */

#include <assert.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <sysinit/sysinit.h>
#include <syscfg/syscfg.h>
#include "host/ble_hs_mbuf.h"
#include "os/os_mbuf.h"
#include "nimble/transport.h"
#include "nimble/transport/hci_h4.h"

#include <rtthread.h>
#include <rtdevice.h>

#define DBG_TAG             "nimble.vhci"
#define DBG_LVL             DBG_INFO
#include "rtdbg.h"

static rt_sem_t rx_sem;
static struct hci_h4_sm hci_h4sm;
static rt_device_t vhci_dev;

static int hci_uart_frame_cb(uint8_t pkt_type, void *data)
{
    switch (pkt_type) {
    case HCI_H4_EVT:
        return ble_transport_to_hs_evt(data);
    case HCI_H4_ACL:
        return ble_transport_to_hs_acl(data);
    default:
        assert(0);
        break;
    }
    return -1;
}

static rt_err_t rx_indicate(rt_device_t dev, rt_size_t size)
{
    rt_sem_release(rx_sem);
    return RT_EOK;
}

static void rtthread_vhci_rx_entry(void *parameter)
{
    uint8_t data[64];
    size_t data_len;

    rx_sem = rt_sem_create("vhci_rx_sem", 0, RT_IPC_FLAG_FIFO);
    RT_ASSERT(rx_sem != RT_NULL);
    rt_device_set_rx_indicate(vhci_dev, rx_indicate);

    while (1)
    {
        rt_sem_take(rx_sem, RT_WAITING_FOREVER);
        data_len = rt_device_read(vhci_dev, 0, data, sizeof(data));
        if (data_len > 0)
        {
            LOG_HEX("Rx: ", 16, data, data_len);
            hci_h4_sm_rx(&hci_h4sm, data, data_len);
        }
    }
}

static int rtthread_vhci_dev_init(void)
{
    if ((vhci_dev = rt_device_find(RT_NIMBLE_VHCI_DEVICE_NAME)) == RT_NULL) {
        LOG_E("vhci device %s not found", RT_NIMBLE_VHCI_DEVICE_NAME);
        return -1;
    }

    if (rt_device_open(vhci_dev, RT_DEVICE_FLAG_RDWR) != RT_EOK) {
        LOG_E("vhci device %s open failed", RT_NIMBLE_VHCI_DEVICE_NAME);
        return -1;
    }

    rt_thread_t rx_thread = rt_thread_create("vhci_rx", rtthread_vhci_rx_entry, RT_NULL, 1024, 25, 10);
    if (rx_thread != RT_NULL) {
        rt_thread_startup(rx_thread);
    } else {
        LOG_E("vhci rx thread create failed");
        return -1;
    }

    return 0;
}

int ble_transport_to_ll_cmd_impl(void *buf)
{
    int res = 0;
    int buf_len = 3 + ((uint8_t *)buf)[2] + 1;
    uint8_t *data = NULL;

    data = rt_malloc(buf_len);
    if (!data) {
        LOG_E("%s: malloc failed", __func__);
        res = -RT_ERROR;
        goto __exit;
    }

    data[0] = HCI_H4_CMD;
    memcpy(&data[1], buf, buf_len - 1);

    LOG_HEX("Tx: ", 16, data, buf_len);
    if (rt_device_write(vhci_dev, 0, data, buf_len) != buf_len) {
        LOG_E("Error writing HCI_H4_CMD data");
        res = -RT_ERROR;
        rt_free(data);
        goto __exit;
    }
    rt_free(data);

    __exit:
    ble_transport_free(buf);

    return res;
}

int ble_transport_to_ll_acl_impl(struct os_mbuf *om)
{
    int res = 0;
    struct os_mbuf *x = om;

    while (x != NULL)
    {
        int data_len = OS_MBUF_PKTLEN(x) + 1;
        uint8_t *data = NULL;

        LOG_D("ble_transport_to_ll_acl_impl: %d bytes", data_len);

        data = rt_malloc(data_len);
        if (!data) {
            LOG_E("%s: malloc failed", __func__);
            res = -RT_ERROR;
            goto __exit;
        }

        data[0] = HCI_H4_ACL;
        res = ble_hs_mbuf_to_flat(x, &data[1], OS_MBUF_PKTLEN(x), NULL);
        if (res) {
            LOG_E("Error copying HCI_H4_ACL data %d", res);
            res = -RT_ERROR;
            rt_free(data);
            goto __exit;
        }

        LOG_HEX("Tx: ", 16, data, data_len);
        if (rt_device_write(vhci_dev, 0, data, data_len) != data_len) {
            LOG_E("Error writing HCI_H4_ACL data");
            res = -RT_ERROR;
            rt_free(data);
            goto __exit;
        }
        rt_free(data);

        x = SLIST_NEXT(x, om_next);
    }

    __exit:
    os_mbuf_free_chain(om);

    return res;
}

static int rtthread_ble_transport_init(void)
{
    int rc;
    SYSINIT_ASSERT_ACTIVE();

    hci_h4_sm_init(&hci_h4sm, &hci_h4_allocs_from_ll, hci_uart_frame_cb);

    rc = rtthread_vhci_dev_init();
    SYSINIT_PANIC_ASSERT(rc == 0);

    return 0;
}
INIT_APP_EXPORT(rtthread_ble_transport_init);
