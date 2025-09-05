/**
 * @file ui_led.h
 * @brief LED控制界面头文件
 *
 * 提供LED控制界面的接口定义，包括灯珠数量设置、颜色控制、特效模式等
 *
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 */

#ifndef __UI_LED_H__
#define __UI_LED_H__

#include "tuya_cloud_types.h"
#include "lvgl.h"
#include "rgb_led.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
************************macro define************************
***********************************************************/
#define MAX_LED_COUNT_UI 64
#define MAX_COLOR_PRESETS 16
#define MAX_EFFECT_PRESETS 8

/***********************************************************
***********************typedef define***********************
***********************************************************/
// LED特效模式枚举
typedef enum {
    LED_EFFECT_OFF = 0,
    LED_EFFECT_SOLID,           // 纯色
    LED_EFFECT_RAINBOW,         // 彩虹渐变
    LED_EFFECT_WAVE,            // 波浪效果
    LED_EFFECT_FIRE,            // 火焰效果
    LED_EFFECT_TWINKLE,         // 闪烁效果
    LED_EFFECT_CHASE,           // 追逐效果
    LED_EFFECT_BREATH,          // 呼吸效果
    LED_EFFECT_MAX
} LED_EFFECT_MODE_E;

// 颜色预设结构
typedef struct {
    char name[16];
    rgb_color_t color;
} LED_COLOR_PRESET_T;

// LED UI主题颜色
typedef struct {
    lv_color_t background;
    lv_color_t text;
    lv_color_t button_bg;
    lv_color_t button_text;
    lv_color_t slider_bg;
    lv_color_t slider_indicator;
    lv_color_t border;
    lv_color_t accent;
} LED_UI_THEME_T;

// LED UI界面结构
typedef struct {
    lv_obj_t *screen;
    lv_obj_t *container;
    lv_obj_t *title_label;
    
    // 灯珠数量控制
    lv_obj_t *count_container;
    lv_obj_t *count_label;
    lv_obj_t *count_slider;
    lv_obj_t *count_value_label;
    
    // 颜色控制
    lv_obj_t *color_container;
    lv_obj_t *color_label;
    lv_obj_t *red_slider;
    lv_obj_t *green_slider;
    lv_obj_t *blue_slider;
    lv_obj_t *red_label;
    lv_obj_t *green_label;
    lv_obj_t *blue_label;
    lv_obj_t *color_preview;
    
    // 亮度控制
    lv_obj_t *brightness_container;
    lv_obj_t *brightness_label;
    lv_obj_t *brightness_slider;
    lv_obj_t *brightness_value_label;
    
    // 特效控制
    lv_obj_t *effect_container;
    lv_obj_t *effect_label;
    lv_obj_t *effect_dropdown;
    
    // 预设颜色
    lv_obj_t *preset_container;
    lv_obj_t *preset_label;
    lv_obj_t *preset_buttons[MAX_COLOR_PRESETS];
    
    // 控制按钮
    lv_obj_t *control_container;
    lv_obj_t *apply_button;
    lv_obj_t *reset_button;
    lv_obj_t *off_button;
    
    // 状态显示
    lv_obj_t *status_container;
    lv_obj_t *status_label;
    lv_obj_t *info_label;
} LED_UI_T;

// LED UI上下文
typedef struct {
    LED_UI_T ui;
    LED_UI_THEME_T theme;
    
    // 当前设置
    uint8_t current_led_count;
    rgb_color_t current_color;
    uint8_t current_brightness;
    LED_EFFECT_MODE_E current_effect;
    
    // 预设颜色
    LED_COLOR_PRESET_T color_presets[MAX_COLOR_PRESETS];
    uint8_t preset_count;
    
    // 特效定时器
    lv_timer_t *effect_timer;
    uint32_t effect_counter;
    
    // 回调函数
    void (*on_led_count_change)(uint8_t count);
    void (*on_color_change)(rgb_color_t color);
    void (*on_brightness_change)(uint8_t brightness);
    void (*on_effect_change)(LED_EFFECT_MODE_E effect);
} LED_UI_CONTEXT_T;

/***********************************************************
********************function declaration********************
***********************************************************/

/**
 * @brief 初始化LED UI界面
 * 
 * @param font UI字体
 * @return int 成功返回0，失败返回-1
 */
int ui_led_init(lv_font_t *font);

/**
 * @brief 显示LED UI界面
 * 
 * @param parent 父对象，NULL表示创建新屏幕
 * @return int 成功返回0，失败返回-1
 */
int ui_led_show(lv_obj_t *parent);

/**
 * @brief 隐藏LED UI界面
 * 
 * @return int 成功返回0，失败返回-1
 */
int ui_led_hide(void);

/**
 * @brief 设置LED数量
 * 
 * @param count LED数量
 * @return int 成功返回0，失败返回-1
 */
int ui_led_set_count(uint8_t count);

/**
 * @brief 设置LED颜色
 * 
 * @param color RGB颜色
 * @return int 成功返回0，失败返回-1
 */
int ui_led_set_color(rgb_color_t color);

/**
 * @brief 设置LED亮度
 * 
 * @param brightness 亮度值(0-100)
 * @return int 成功返回0，失败返回-1
 */
int ui_led_set_brightness(uint8_t brightness);

/**
 * @brief 设置LED特效模式
 * 
 * @param effect 特效模式
 * @return int 成功返回0，失败返回-1
 */
int ui_led_set_effect(LED_EFFECT_MODE_E effect);

/**
 * @brief 添加颜色预设
 * 
 * @param name 预设名称
 * @param color 预设颜色
 * @return int 成功返回0，失败返回-1
 */
int ui_led_add_color_preset(const char *name, rgb_color_t color);

/**
 * @brief 获取LED UI屏幕对象
 * 
 * @return lv_obj_t* 屏幕对象指针
 */
lv_obj_t *ui_led_get_screen(void);

/**
 * @brief 注册LED变化回调函数
 * 
 * @param count_cb LED数量变化回调
 * @param color_cb 颜色变化回调
 * @param brightness_cb 亮度变化回调
 * @param effect_cb 特效变化回调
 * @return int 成功返回0，失败返回-1
 */
int ui_led_register_callbacks(
    void (*count_cb)(uint8_t count),
    void (*color_cb)(rgb_color_t color),
    void (*brightness_cb)(uint8_t brightness),
    void (*effect_cb)(LED_EFFECT_MODE_E effect)
);

#ifdef __cplusplus
}
#endif

#endif // __UI_LED_H__
