/**
 * @file led_pixel_breath.h
 * @brief LED pixel strip control APIs (breath + steady)
 * @version 0.1
 * @date 2026-05-08
 * @copyright Copyright (c) 2026 Tuya Inc. All Rights Reserved.
 */
#ifndef __LED_PIXEL_BREATH_H__
#define __LED_PIXEL_BREATH_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(ENABLE_LEDS_PIXEL) && (ENABLE_LEDS_PIXEL)

/**
 * @brief Porting notes for different boards/strips
 * @note The implementation supports compile-time overrides of:
 *       `LEDS_PIXEL_NAME`, `LED_PIXEL_HW_ORDER_GRB`, `LED_PIXELS_TOTAL_NUM`,
 *       `LED_PIXEL_COLOR_RES`, `LED_PIXEL_BREATH_STEP`, and `LED_PIXEL_BREATH_MS`.
 *       Override them with compiler defines to adapt to new hardware quickly.
 */

/**
 * @brief Register LED pixel hardware driver
 * @return OPRT_OK on success, error code otherwise
 * @note This API is intended to be called from board-level
 *       `board_register_hardware()` during hardware registration stage.
 */
OPERATE_RET led_pixel_register_hardware(void);

/**
 * @brief Start white breathing
 * @return OPRT_OK on success, error code otherwise
 * @note Call after board_register_hardware() and tal_sw_timer_init().
 */
OPERATE_RET led_pixel_breath_start_white(void);

/**
 * @brief Start breathing with a custom RGB hex color
 * @param[in] rgb_hex six-digit RGB hex string, optionally prefixed with '#'
 * @return OPRT_OK on success, error code otherwise
 */
OPERATE_RET led_pixel_breath_start_blue(const char *rgb_hex);

/**
 * @brief Apply RGB hex color with brightness and mode
 * @param[in] rgb_hex six-digit RGB hex string, optionally prefixed with '#'
 * @param[in] brightness_pct brightness percentage in range 0..100
 * @param[in] is_breath true for breath mode, false for steady mode
 * @return OPRT_OK on success, error code otherwise
 */
OPERATE_RET led_pixel_apply_rgb_mode(const char *rgb_hex, uint8_t brightness_pct, bool is_breath);

/**
 * @brief Start purple breathing (red + blue)
 * @return OPRT_OK on success, error code otherwise
 */
OPERATE_RET led_pixel_breath_start_purple(void);

/**
 * @brief Start red breathing
 * @return OPRT_OK on success, error code otherwise
 */
OPERATE_RET led_pixel_breath_start_red(void);

/**
 * @brief Start green breathing
 * @return OPRT_OK on success, error code otherwise
 */
OPERATE_RET led_pixel_breath_start_green(void);

/**
 * @brief Start cyan breathing
 * @return OPRT_OK on success, error code otherwise
 */
OPERATE_RET led_pixel_breath_start_cyan(void);

/**
 * @brief Start yellow breathing
 * @return OPRT_OK on success, error code otherwise
 */
OPERATE_RET led_pixel_breath_start_yellow(void);

/**
 * @brief Start orange breathing
 * @return OPRT_OK on success, error code otherwise
 */
OPERATE_RET led_pixel_breath_start_orange(void);

/**
 * @brief Start pink breathing
 * @return OPRT_OK on success, error code otherwise
 */
OPERATE_RET led_pixel_breath_start_pink(void);

/**
 * @brief Stop breath timer and turn all pixels off
 * @return OPRT_OK on success, error code otherwise
 */
OPERATE_RET led_pixel_breath_stop(void);

#endif /* ENABLE_LEDS_PIXEL */

#ifdef __cplusplus
}
#endif

#endif /* __LED_PIXEL_BREATH_H__ */
