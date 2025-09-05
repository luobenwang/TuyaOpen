#include "ui_cooking.h"
#include "lvgl.h"
#include "tuya_log.h"
#include "tuya_error_code.h"
#include "emotion_images/emotion_huoguo_image.c"

// Font declarations
LV_FONT_DECLARE(font_puhui_18_2);

/***********************************************************
********************macro define****************************
***********************************************************/
#define TAG "UI_COOKING"

/***********************************************************
********************typedef define**************************
***********************************************************/
typedef struct {
    lv_obj_t *screen;
    lv_obj_t *content;
    lv_obj_t *title_label;
    lv_obj_t *huoguo_image;
    lv_obj_t *back_btn;
    lv_obj_t *back_label;
    lv_obj_t *prev_screen; // for returning to chat
} APP_COOKING_UI_T;

/***********************************************************
********************function declaration********************
***********************************************************/
static void on_back_to_chat(lv_event_t *e);

/***********************************************************
***********************variable define**********************
***********************************************************/
static APP_COOKING_UI_T sg_ui = {0};

/***********************************************************
********************function implementation*****************
***********************************************************/

static void on_back_to_chat(lv_event_t *e)
{
    (void)e;
    if (sg_ui.prev_screen) {
        lv_scr_load_anim(sg_ui.prev_screen, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 200, 0, false);
    }
}

void ui_cooking_show(lv_obj_t *prev_screen)
{
    sg_ui.prev_screen = prev_screen;
    
    // Create screen
    sg_ui.screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(sg_ui.screen, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(sg_ui.screen, LV_OPA_COVER, 0);
    
    // Set font for the screen to support Chinese characters
    lv_obj_set_style_text_font(sg_ui.screen, &font_puhui_18_2, 0);
    
    // Create content container
    sg_ui.content = lv_obj_create(sg_ui.screen);
    lv_obj_set_size(sg_ui.content, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_opa(sg_ui.content, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(sg_ui.content, 0, 0);
    lv_obj_set_style_pad_all(sg_ui.content, 20, 0);
    lv_obj_center(sg_ui.content);
    
    // Create title label
    sg_ui.title_label = lv_label_create(sg_ui.content);
    lv_label_set_text(sg_ui.title_label, "美食制作中");
    lv_obj_set_style_text_color(sg_ui.title_label, lv_color_hex(0xFF0000), 0); // Red color
    // lv_obj_set_style_text_font(sg_ui.title_label, &font_puhui_18_2, 0); // Use default font
    lv_obj_align(sg_ui.title_label, LV_ALIGN_TOP_MID, 0, 30);
    
    // Create huoguo image
    sg_ui.huoguo_image = lv_image_create(sg_ui.content);
    
    // Build image descriptor for huoguo image
    static lv_image_dsc_t img_dsc;
    memset(&img_dsc, 0, sizeof(img_dsc));
    img_dsc.header.magic = LV_IMAGE_HEADER_MAGIC;
    img_dsc.header.cf = LV_COLOR_FORMAT_RGB565;
    img_dsc.header.flags = 0; // not compressed
    img_dsc.header.w = emotion_huoguo_image_width;
    img_dsc.header.h = emotion_huoguo_image_height;
    img_dsc.header.stride = (uint32_t)emotion_huoguo_image_width * 2; // RGB565: 2 bytes per pixel
    img_dsc.data_size = (uint32_t)img_dsc.header.stride * emotion_huoguo_image_height;
    img_dsc.data = (const uint8_t *)emotion_huoguo_image_data;
    
    // Apply to image widget
    lv_image_set_src(sg_ui.huoguo_image, &img_dsc);
    lv_obj_align(sg_ui.huoguo_image, LV_ALIGN_CENTER, 0, 0);
    
    // Create back button
    sg_ui.back_btn = lv_btn_create(sg_ui.content);
    lv_obj_set_size(sg_ui.back_btn, 100, 40);
    lv_obj_align(sg_ui.back_btn, LV_ALIGN_BOTTOM_MID, 0, -30);
    lv_obj_set_style_bg_color(sg_ui.back_btn, lv_color_hex(0x666666), 0);
    
    sg_ui.back_label = lv_label_create(sg_ui.back_btn);
    lv_label_set_text(sg_ui.back_label, "返回");
    lv_obj_set_style_text_color(sg_ui.back_label, lv_color_hex(0xFFD700), 0); // Gold color
    lv_obj_center(sg_ui.back_label);
    lv_obj_add_event_cb(sg_ui.back_btn, on_back_to_chat, LV_EVENT_CLICKED, NULL);
    
    // Load screen
    lv_scr_load_anim(sg_ui.screen, LV_SCR_LOAD_ANIM_MOVE_LEFT, 200, 0, false);
}
