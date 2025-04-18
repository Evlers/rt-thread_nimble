/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Date           Author       Notes
 * 2018-12-29     ChenYong     first implementation
 * 2022-05-20     Jackistang   add ble host auto start
 * 2025-03-01     Evlers       only hci transmission is supported
 */

#ifndef __NUMBLE_CONFIG_H__
#define __NUMBLE_CONFIG_H__

#include <rtconfig.h>
#include "os/util.h"
#include "console/console.h"


/**
 * Bluetooth role support
 */
#ifdef RT_NIMBLE_ROLE_PERIPHERAL
#define MYNEWT_VAL_BLE_ROLE_PERIPHERAL           (1)
#endif

#ifdef RT_NIMBLE_ROLE_CENTRAL
#define MYNEWT_VAL_BLE_ROLE_CENTRAL              (1)
#endif

#ifdef RT_NIMBLE_ROLE_BROADCASTER
#define MYNEWT_VAL_BLE_ROLE_BROADCASTER          (1)
#endif

#ifdef RT_NIMBLE_ROLE_OBSERVER
#define MYNEWT_VAL_BLE_ROLE_OBSERVER             (1)
#endif

#ifdef RT_NIMBLE_MAX_CONNECTIONS
#define MYNEWT_VAL_BLE_MAX_CONNECTIONS           (RT_NIMBLE_MAX_CONNECTIONS)
#endif

#ifdef RT_NIMBLE_MULTI_ADV_INSTANCES
#define MYNEWT_VAL_BLE_MULTI_ADV_INSTANCES       (RT_NIMBLE_MULTI_ADV_INSTANCES)
#endif

#ifdef RT_NIMBLE_WHITELIST
#define MYNEWT_VAL_BLE_WHITELIST                 (1)
#endif

#ifdef RT_NIMBLE_EXT_ADV
#define MYNEWT_VAL_BLE_EXT_ADV                   (1)
#endif

#ifdef RT_NIMBLE_EXT_ADV_MAX_SIZE
#define MYNEWT_VAL_BLE_EXT_ADV_MAX_SIZE          (RT_NIMBLE_EXT_ADV_MAX_SIZE)
#endif

/**
 * Host Stack Configuration
 */
#ifdef RT_NIMBLE_HOST_THREAD_STACK_SIZE
#define MYNEWT_VAL_BLE_HOST_THREAD_STACK_SIZE    (RT_NIMBLE_HOST_THREAD_STACK_SIZE)
#endif

#ifdef RT_NIMBLE_HOST_THREAD_PRIORITY
#define MYNEWT_VAL_BLE_HOST_THREAD_PRIORITY      (RT_NIMBLE_HOST_THREAD_PRIORITY)
#endif

#define MYNEWT_VAL_BLE_HS_AUTO_START 1

/**
 * Mesh Configuration
 */
#ifdef RT_NIMBLE_MESH
#define MYNEWT_VAL_BLE_MESH                      (1)
#endif

#ifdef RT_NIMBLE_MESH_DEVICE_NAME
#define MYNEWT_VAL_BLE_MESH_DEVICE_NAME          RT_NIMBLE_MESH_DEVICE_NAME
#endif

#ifdef RT_NIMBLE_MESH_ADV_THREAD_STACK_SIZE
#define MYNEWT_VAL_BLE_MESH_ADV_THREAD_STACK_SIZE (RT_NIMBLE_MESH_ADV_THREAD_STACK_SIZE)
#endif

#ifdef RT_NIMBLE_MESH_ADV_THREAD_PRIORITY
#define MYNEWT_VAL_BLE_MESH_ADV_THREAD_PRIORITY  (RT_NIMBLE_MESH_ADV_THREAD_PRIORITY)
#endif

#ifdef RT_NIMBLE_MESH_CFG_CLI
#define BLE_MESH_CFG_CLI                         (1)
#endif


#endif
