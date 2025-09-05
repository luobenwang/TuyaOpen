/**
 * @file ui_rgb_control.h
 * @brief RGB LED control UI interface header
 *
 * This header file provides the interface for the RGB LED control UI.
 *
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 */

#ifndef UI_RGB_CONTROL_H
#define UI_RGB_CONTROL_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Show RGB control UI screen
 */
void ui_rgb_control_show(void);

/**
 * @brief Hide RGB control UI screen
 */
void ui_rgb_control_hide(void);

#ifdef __cplusplus
}
#endif

#endif // UI_RGB_CONTROL_H
