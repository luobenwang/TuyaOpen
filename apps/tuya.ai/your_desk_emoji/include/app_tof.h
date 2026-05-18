/**
 * @file app_tof.h
 * @brief VL53L0X time-of-flight ranging on I2C0 (TOF owns bus init by default)
 * @version 1.1
 * @date 2026-05-11
 * @copyright Copyright (c) 2026 Tuya Inc. All Rights Reserved.
 */

#ifndef __APP_TOF_H__
#define __APP_TOF_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Distance callback (millimeters from VL53L0X single ranging)
 * @param[in] distance_mm range in millimeters
 * @return none
 */
typedef VOID_T (*APP_TOF_CB_T)(UINT16_T distance_mm);

/**
 * @brief Start VL53L0X background ranging thread
 * @param[in] cb optional user callback; may be NULL to only log distance
 * @return OPRT_OK on success, OPRT_COM_ERROR if sensor not detected
 * @note Default build: performs pinmux + tkl_i2c_init on I2C0. Set compile-time
 *       APP_TOF_SHARE_I2C0_BUS to 1 only if gesture already initialized the same bus.
 */
OPERATE_RET app_tof_init(APP_TOF_CB_T cb);

/**
 * @brief Default handler: distance log, LVGL on band change; NEAR->FAR records drink + AI text like GPIO path
 * @param[in] distance_mm range in millimeters
 * @return none
 */
VOID app_tof_default_proximity_handler(UINT16_T distance_mm);

#ifdef __cplusplus
}
#endif

#endif /* __APP_TOF_H__ */
