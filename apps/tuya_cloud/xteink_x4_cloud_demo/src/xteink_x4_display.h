/**
 * @file xteink_x4_display.h
 * @brief Lightweight 1bpp EPD display for XTEINK X4 (no LVGL).
 * @version 2.0
 * @date 2026-07-16
 * @copyright Copyright (c) 2026 Tuya Inc. All Rights Reserved.
 */
#ifndef __XTEINK_X4_DISPLAY_H__
#define __XTEINK_X4_DISPLAY_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Start lightweight EPD UI in a background thread.
 * @return OPRT_OK on success
 * @note Uses direct 1bpp rendering (~48 KB static FB only, no LVGL heap).
 */
OPERATE_RET xteink_x4_display_start(void);

/**
 * @brief Update cloud status text shown on the dashboard footer.
 * @param[in] status Short status string (e.g. "MQTT connected")
 * @return none
 */
void xteink_x4_display_set_cloud_status(const char *status);

/**
 * @brief Block until EPD hardware init and first splash frame are done.
 * @param[in] timeout_ms Max wait in milliseconds (0 = no wait, returns immediately)
 * @return OPRT_OK when ready, OPRT_TIMEOUT on timeout, OPRT_COM_ERROR if display not started
 * @note Call after xteink_x4_display_start(). Use before WiFi/BLE init so EPD init runs with full heap.
 */
OPERATE_RET xteink_x4_display_wait_ready(uint32_t timeout_ms);

/**
 * @brief Flash power-off screen then enter deep sleep (EXT1 wake on PWR).
 * @return none
 * @note Safe to call from IoT event callback after factory reset completes.
 */
void xteink_x4_display_enter_deep_sleep(void);

/**
 * @brief App hook for PWR hold >= 3s (implemented in tuya_main.c).
 * @return none
 * @note Cloud build: tuya_iot_reset() then sleep on RESET_COMPLETE. Display-only: sleep now.
 */
void xteink_x4_app_on_pwr_long_press(void);

#ifdef __cplusplus
}
#endif

#endif /* __XTEINK_X4_DISPLAY_H__ */
