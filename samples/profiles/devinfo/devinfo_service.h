/*
 * Copyright (c) 2006-2025 LGT Development Team
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-03-01     Evlers       first implementation
 */

#ifndef _DEVINFO_SERVICE_H_
#define _DEVINFO_SERVICE_H_

#ifdef __cplusplus
extern "C" {
#endif

/* service and characteristic */
#define GATT_DEVICE_INFO_UUID                   0x180A
#define GATT_MANUFACTURER_NAME_UUID             0x2A29
#define GATT_MODEL_NUMBER_UUID                  0x2A24

void devinfo_service_set_manuf_name (const char *name);
void devinfo_service_set_model_num (const char *num);
int devinfo_service_register (void);

#ifdef __cplusplus
}
#endif

#endif /* _DEVINFO_SERVICE_H_ */
