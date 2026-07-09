/**
 * @file xteink_x4_display.h
 * @brief LVGL-on-EPD display for XTEINK X4 (from lvgl_demo UI).
 * @version 1.0
 * @date 2026-07-08
 * @copyright Copyright (c) 2026 Tuya Inc. All Rights Reserved.
 */
#ifndef __XTEINK_X4_DISPLAY_H__
#define __XTEINK_X4_DISPLAY_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Start LVGL + EPD UI in a background thread.
 * @return OPRT_OK on success
 * @note Runs the same boot sequence and dashboard as boards/.../lvgl_demo.
 */
OPERATE_RET xteink_x4_display_start(void);

/**
 * @brief Update cloud status text shown on the dashboard footer.
 * @param[in] status Short status string (e.g. "MQTT connected")
 * @return none
 */
void xteink_x4_display_set_cloud_status(const char *status);

#ifdef __cplusplus
}
#endif

#endif /* __XTEINK_X4_DISPLAY_H__ */
