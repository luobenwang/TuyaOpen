/**
 * @file ui_rgb_control.c
 * @brief RGB LED control UI interface
 *
 * This source file provides the UI for controlling RGB LED with brightness
 * and color sliders.
 *
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 */

#include "tuya_cloud_types.h"

#include "ui_display.h"
#include "rgb_led.h"
#include "font_awesome_symbols.h"
#include "lvgl.h"
#include <string.h>

// Font declarations
LV_FONT_DECLARE(font_puhui_18_2);

/***********************************************************
************************macro define************************
***********************************************************/
#define GPIO_P10_PIN 10  // P10 pin corresponds to GPIO 10
#define DEFAULT_LED_COUNT 7

/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef struct {
    lv_obj_t *container;
    lv_obj_t *top_bar;
    lv_obj_t *back_btn;
    lv_obj_t *title_label;
    lv_obj_t *content;
    lv_obj_t *brightness_slider;
    lv_obj_t *brightness_label;
    lv_obj_t *color_slider;
    lv_obj_t *color_label;
    lv_obj_t *preview_area;
    lv_obj_t *preview_label;
    lv_obj_t *status_label;
} RGB_CONTROL_UI_T;

typedef struct {
    RGB_CONTROL_UI_T ui;
    rgb_led_config_t led_config;
    lv_timer_t *update_timer;
} RGB_CONTROL_APP_T;

/***********************************************************
********************function declaration********************
***********************************************************/
static void ui_rgb_build_screen(void);
static void on_back_to_chat(lv_event_t *e);
static void on_brightness_changed(lv_event_t *e);
static void on_color_changed(lv_event_t *e);
static void update_preview(void);
static void update_led(void);
static uint32_t hue_to_rgb(uint16_t hue);
void ui_rgb_control_hide(void);

/***********************************************************
***********************variable define**********************
***********************************************************/
static RGB_CONTROL_APP_T sg_rgb_ui = {0};

/***********************************************************
***********************function define**********************
***********************************************************/
static uint32_t hue_to_rgb(uint16_t hue)
{
    // Convert HSV hue (0-360) to RGB
    uint8_t sector = hue / 60;
    uint8_t offset = hue % 60;
    uint8_t p = 0;
    uint8_t q = (255 * (60 - offset)) / 60;
    uint8_t t = (255 * offset) / 60;
    
    uint8_t r, g, b;
    switch (sector) {
        case 0: r = 255; g = t; b = p; break;
        case 1: r = q; g = 255; b = p; break;
        case 2: r = p; g = 255; b = t; break;
        case 3: r = p; g = q; b = 255; break;
        case 4: r = t; g = p; b = 255; break;
        default: r = 255; g = p; b = q; break;
    }
    
    return (r << 16) | (g << 8) | b;
}

static void update_preview(void)
{
    if (sg_rgb_ui.ui.preview_area == NULL) {
        return;
    }
    
    // Get current color from slider
    int32_t hue = lv_slider_get_value(sg_rgb_ui.ui.color_slider);
    uint32_t rgb = hue_to_rgb(hue);
    
    // Update preview area background color
    lv_color_t color = lv_color_hex(rgb);
    lv_obj_set_style_bg_color(sg_rgb_ui.ui.preview_area, color, 0);
    
    // Update preview label text
    char preview_text[64];
    snprintf(preview_text, sizeof(preview_text), "颜色: %d°\n亮度: %d%%", 
             hue, sg_rgb_ui.led_config.brightness);
    lv_label_set_text(sg_rgb_ui.ui.preview_label, preview_text);
}

static void update_led(void)
{
    // Get current values from UI
    int32_t brightness = lv_slider_get_value(sg_rgb_ui.ui.brightness_slider);
    int32_t hue = lv_slider_get_value(sg_rgb_ui.ui.color_slider);
    
    // Convert hue to RGB
    uint32_t rgb = hue_to_rgb(hue);
    rgb_color_t color;
    color.red = (rgb >> 16) & 0xFF;
    color.green = (rgb >> 8) & 0xFF;
    color.blue = rgb & 0xFF;
    
    // Update LED
    rgb_led_set_color_brightness(color, brightness);
    
    // Update local config
    sg_rgb_ui.led_config.color = color;
    sg_rgb_ui.led_config.brightness = brightness;
}

static void on_brightness_changed(lv_event_t *e)
{
    lv_obj_t *slider = lv_event_get_target(e);
    int32_t value = lv_slider_get_value(slider);
    
    // Update brightness label
    char label_text[32];
    snprintf(label_text, sizeof(label_text), "亮度: %d%%", value);
    lv_label_set_text(sg_rgb_ui.ui.brightness_label, label_text);
    
    // Update LED
    update_led();
    update_preview();
}

static void on_color_changed(lv_event_t *e)
{
    lv_obj_t *slider = lv_event_get_target(e);
    int32_t value = lv_slider_get_value(slider);
    
    // Update color label
    char label_text[32];
    snprintf(label_text, sizeof(label_text), "颜色: %d°", value);
    lv_label_set_text(sg_rgb_ui.ui.color_label, label_text);
    
    // Update LED
    update_led();
    update_preview();
}

static void on_back_to_chat(lv_event_t *e)
{
    (void)e;
    
    // For now, just hide the RGB control UI
    // In a real implementation, you would load the chat screen here
    ui_rgb_control_hide();
}

static void ui_rgb_build_screen(void)
{
    lv_obj_t *screen = lv_screen_active();
    
    // Container
    sg_rgb_ui.ui.container = lv_obj_create(screen);
    lv_obj_set_size(sg_rgb_ui.ui.container, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_flex_flow(sg_rgb_ui.ui.container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(sg_rgb_ui.ui.container, 0, 0);
    lv_obj_set_style_border_width(sg_rgb_ui.ui.container, 0, 0);
    lv_obj_set_style_bg_color(sg_rgb_ui.ui.container, lv_color_white(), 0);
    
    // Top bar
    sg_rgb_ui.ui.top_bar = lv_obj_create(sg_rgb_ui.ui.container);
    lv_obj_set_size(sg_rgb_ui.ui.top_bar, LV_HOR_RES, 50);
    lv_obj_set_flex_flow(sg_rgb_ui.ui.top_bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_radius(sg_rgb_ui.ui.top_bar, 0, 0);
    lv_obj_set_style_bg_color(sg_rgb_ui.ui.top_bar, lv_color_hex(0x2196F3), 0);
    
    // Back button
    sg_rgb_ui.ui.back_btn = lv_btn_create(sg_rgb_ui.ui.top_bar);
    lv_obj_set_size(sg_rgb_ui.ui.back_btn, 80, 40);
    lv_obj_t *back_label = lv_label_create(sg_rgb_ui.ui.back_btn);
    lv_label_set_text(back_label, "返回");
    lv_obj_center(back_label);
    lv_obj_add_event_cb(sg_rgb_ui.ui.back_btn, on_back_to_chat, LV_EVENT_CLICKED, NULL);
    
    // Title
    sg_rgb_ui.ui.title_label = lv_label_create(sg_rgb_ui.ui.top_bar);
    lv_label_set_text(sg_rgb_ui.ui.title_label, "RGB LED 控制");
    lv_obj_set_style_text_color(sg_rgb_ui.ui.title_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(sg_rgb_ui.ui.title_label, &font_puhui_18_2, 0);
    lv_obj_align(sg_rgb_ui.ui.title_label, LV_ALIGN_CENTER, 0, 0);
    
    // Content
    sg_rgb_ui.ui.content = lv_obj_create(sg_rgb_ui.ui.container);
    lv_obj_set_width(sg_rgb_ui.ui.content, LV_HOR_RES);
    lv_obj_set_flex_grow(sg_rgb_ui.ui.content, 1);
    lv_obj_set_flex_flow(sg_rgb_ui.ui.content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(sg_rgb_ui.ui.content, 20, 0);
    lv_obj_set_style_bg_color(sg_rgb_ui.ui.content, lv_color_white(), 0);
    
    // Brightness slider
    sg_rgb_ui.ui.brightness_label = lv_label_create(sg_rgb_ui.ui.content);
    lv_label_set_text(sg_rgb_ui.ui.brightness_label, "亮度: 50%");
    lv_obj_set_style_text_font(sg_rgb_ui.ui.brightness_label, &font_puhui_18_2, 0);
    
    sg_rgb_ui.ui.brightness_slider = lv_slider_create(sg_rgb_ui.ui.content);
    lv_obj_set_size(sg_rgb_ui.ui.brightness_slider, LV_HOR_RES - 40, 20);
    lv_slider_set_range(sg_rgb_ui.ui.brightness_slider, 0, 100);
    lv_slider_set_value(sg_rgb_ui.ui.brightness_slider, 50, LV_ANIM_OFF);
    lv_obj_add_event_cb(sg_rgb_ui.ui.brightness_slider, on_brightness_changed, LV_EVENT_VALUE_CHANGED, NULL);
    
    // Color slider
    sg_rgb_ui.ui.color_label = lv_label_create(sg_rgb_ui.ui.content);
    lv_label_set_text(sg_rgb_ui.ui.color_label, "颜色: 0°");
    lv_obj_set_style_text_font(sg_rgb_ui.ui.color_label, &font_puhui_18_2, 0);
    
    sg_rgb_ui.ui.color_slider = lv_slider_create(sg_rgb_ui.ui.content);
    lv_obj_set_size(sg_rgb_ui.ui.color_slider, LV_HOR_RES - 40, 20);
    lv_slider_set_range(sg_rgb_ui.ui.color_slider, 0, 360);
    lv_slider_set_value(sg_rgb_ui.ui.color_slider, 0, LV_ANIM_OFF);
    lv_obj_add_event_cb(sg_rgb_ui.ui.color_slider, on_color_changed, LV_EVENT_VALUE_CHANGED, NULL);
    
    // Preview area
    sg_rgb_ui.ui.preview_area = lv_obj_create(sg_rgb_ui.ui.content);
    lv_obj_set_size(sg_rgb_ui.ui.preview_area, LV_HOR_RES - 40, 100);
    lv_obj_set_style_radius(sg_rgb_ui.ui.preview_area, 10, 0);
    lv_obj_set_style_border_width(sg_rgb_ui.ui.preview_area, 2, 0);
    lv_obj_set_style_border_color(sg_rgb_ui.ui.preview_area, lv_color_black(), 0);
    lv_obj_set_style_bg_color(sg_rgb_ui.ui.preview_area, lv_color_hex(0xFF0000), 0);
    
    sg_rgb_ui.ui.preview_label = lv_label_create(sg_rgb_ui.ui.preview_area);
    lv_label_set_text(sg_rgb_ui.ui.preview_label, "颜色: 0°\n亮度: 50%");
    lv_obj_set_style_text_color(sg_rgb_ui.ui.preview_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(sg_rgb_ui.ui.preview_label, &font_puhui_18_2, 0);
    lv_obj_center(sg_rgb_ui.ui.preview_label);
    
    // Status label
    sg_rgb_ui.ui.status_label = lv_label_create(sg_rgb_ui.ui.content);
    lv_label_set_text(sg_rgb_ui.ui.status_label, "RGB LED 已连接");
    lv_obj_set_style_text_font(sg_rgb_ui.ui.status_label, &font_puhui_18_2, 0);
    lv_obj_set_style_text_color(sg_rgb_ui.ui.status_label, lv_color_hex(0x4CAF50), 0);
    
    // // Initialize LED
    if (rgb_led_init(GPIO_P10_PIN, DEFAULT_LED_COUNT) == 0) {
        lv_label_set_text(sg_rgb_ui.ui.status_label, "RGB LED 初始化成功");
        // Set initial LED state
        update_led();
    } else {
        lv_label_set_text(sg_rgb_ui.ui.status_label, "RGB LED 初始化失败");
        lv_obj_set_style_text_color(sg_rgb_ui.ui.status_label, lv_color_hex(0xF44336), 0);
    }
}

void ui_rgb_control_show(void)
{
    // Build the screen
    ui_rgb_build_screen();
}

void ui_rgb_control_hide(void)
{
    if (sg_rgb_ui.ui.container) {
        lv_obj_del(sg_rgb_ui.ui.container);
        sg_rgb_ui.ui.container = NULL;
    }
}
