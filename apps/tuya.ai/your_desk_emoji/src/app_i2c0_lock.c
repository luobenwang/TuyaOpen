/**
 * @file app_i2c0_lock.c
 * @brief Mutex for shared I2C0 between gesture and TOF drivers
 * @version 1.0
 * @date 2026-05-11
 * @copyright Copyright (c) 2026 Tuya Inc. All Rights Reserved.
 */

#include "app_i2c0_lock.h"

#include "tal_api.h"
#include "tal_mutex.h"

/* ---------------------------------------------------------------------------
 * File scope variables
 * --------------------------------------------------------------------------- */
STATIC MUTEX_HANDLE s_i2c0_mtx = NULL;

/* ---------------------------------------------------------------------------
 * Function implementations
 * --------------------------------------------------------------------------- */
/**
 * @brief Create I2C0 bus mutex if not already created
 * @return OPRT_OK on success
 */
OPERATE_RET app_i2c0_lock_init(void)
{
    if (s_i2c0_mtx != NULL) {
        return OPRT_OK;
    }
    return tal_mutex_create_init(&s_i2c0_mtx);
}

/**
 * @brief Lock I2C0 bus
 * @return none
 */
VOID app_i2c0_lock(void)
{
    if (s_i2c0_mtx != NULL) {
        (void)tal_mutex_lock(s_i2c0_mtx);
    }
}

/**
 * @brief Unlock I2C0 bus
 * @return none
 */
VOID app_i2c0_unlock(void)
{
    if (s_i2c0_mtx != NULL) {
        (void)tal_mutex_unlock(s_i2c0_mtx);
    }
}
