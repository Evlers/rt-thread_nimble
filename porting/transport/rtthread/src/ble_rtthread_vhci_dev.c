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
#if DBG_LVL >= DBG_LOG
    static uint8_t data[512];
#else
    static uint8_t data[64];
#endif /* DBG_LVL >= DBG_LOG */
    size_t data_len;

    rx_sem = rt_sem_create("vhci_rx_sem", 0, RT_IPC_FLAG_FIFO);
    RT_ASSERT(rx_sem != RT_NULL);
    rt_device_set_rx_indicate(vhci_dev, rx_indicate);

    while (1)
    {
        rt_sem_take(rx_sem, RT_WAITING_FOREVER);
        while ((data_len = rt_device_read(vhci_dev, 0, data, sizeof(data))) > 0)
        {
#if DBG_LVL >= DBG_LOG
            /* format: TAG rx(<type>) */
            char head[sizeof(DBG_TAG) + sizeof("rx") + sizeof("none") + sizeof("()")];
            char *type;
            switch (data[0])
            {
                case HCI_H4_CMD: type = "cmd"; break;
                case HCI_H4_ACL: type = "acl"; break;
                case HCI_H4_EVT: type = "evt"; break;
                case HCI_H4_ISO: type = "iso"; break;
                default: type = "none"; break;
            }
            rt_snprintf(head, sizeof(head), "%s rx(%s)", DBG_TAG, type);
            LOG_HEX(head, 16, data, data_len);
#endif /* DBG_LVL >= DBG_LOG */
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

    LOG_HEX(DBG_TAG " tx(cmd)", 16, data, buf_len);
    if (rt_device_write(vhci_dev, 0, data, buf_len) != buf_len) {
        LOG_E("error writing cmd data");
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
    int data_len = OS_MBUF_PKTLEN(om) + 1;
    uint8_t *data = NULL;

    data = rt_malloc(data_len);
    if (!data) {
        LOG_E("%s: malloc failed", __func__);
        res = -RT_ERROR;
        goto __exit;
    }

    data[0] = HCI_H4_ACL;
    res = ble_hs_mbuf_to_flat(om, &data[1], OS_MBUF_PKTLEN(om), NULL);
    if (res) {
        LOG_E("error copying acl data %d", res);
        res = -RT_ERROR;
        rt_free(data);
        goto __exit;
    }

    LOG_HEX(DBG_TAG " tx(acl)", 16, data, data_len);
    if (rt_device_write(vhci_dev, 0, data, data_len) != data_len) {
        LOG_E("error writing acl data");
        res = -RT_ERROR;
        rt_free(data);
        goto __exit;
    }
    rt_free(data);

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
