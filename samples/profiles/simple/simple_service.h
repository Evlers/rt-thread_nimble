/*
 * Copyright (c) 2006-2025 LGT Development Team
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-03-01     Evlers       first implementation
 */

#ifndef _SIMPLE_SERVICE_H_
#define _SIMPLE_SERVICE_H_

#ifdef __cplusplus
extern "C" {
#endif

/* service and characteristic */
#define GATT_SIMPLE_SERVICE_UUID                0xFFF0
#define GATT_SIMPLE_NOTIFY_UUID                 0xFFF1
#define GATT_SIMPLE_WRITE_UUID                  0xFFF2

typedef int (*simple_write_cb_t) (void *data, uint16_t len);

int simple_service_notify (uint16_t conn_handle, void *data, uint16_t len);
int simple_service_register (simple_write_cb_t cb);

#ifdef __cplusplus
}
#endif

#endif /* _SIMPLE_SERVICE_H_ */
