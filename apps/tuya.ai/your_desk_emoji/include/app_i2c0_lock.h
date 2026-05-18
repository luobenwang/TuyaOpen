/**
 * @file app_i2c0_lock.h
 * @brief Mutex for shared I2C0 between gesture (PAJ7620) and TOF (VL53L0X)
 * @version 1.0
 * @date 2026-05-11
 * @copyright Copyright (c) 2026 Tuya Inc. All Rights Reserved.
 */

#ifndef __APP_I2C0_LOCK_H__
#define __APP_I2C0_LOCK_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create I2C0 bus mutex if not already created
 * @return OPRT_OK on success
 */
OPERATE_RET app_i2c0_lock_init(void);

/**
 * @brief Lock I2C0 bus (nested lock not supported)
 * @return none
 */
VOID app_i2c0_lock(void);

/**
 * @brief Unlock I2C0 bus
 * @return none
 */
VOID app_i2c0_unlock(void);

#ifdef __cplusplus
}
#endif

#endif /* __APP_I2C0_LOCK_H__ */
