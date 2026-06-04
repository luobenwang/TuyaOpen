/**
 * @file xiao_ssd1306_ui.h
 * @brief Clock and AI reply UI for XIAO ESP32S3 SSD1306 OLED.
 * @version 1.0
 * @date 2026-06-02
 * @copyright Copyright (c) Tuya Inc.
 */
#ifndef __XIAO_SSD1306_UI_H__
#define __XIAO_SSD1306_UI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "tuya_cloud_types.h"

/**
 * @brief Register SSD1306 clock + AI text UI with ai_ui_manage.
 * @return OPRT_OK on success
 */
OPERATE_RET xiao_ssd1306_ui_register(void);

#ifdef __cplusplus
}
#endif

#endif /* __XIAO_SSD1306_UI_H__ */
