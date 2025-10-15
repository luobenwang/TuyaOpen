/**
 * @file ui_control.c
 * @brief Implementation of the control UI interface
 *
 * This source file provides the implementation for a control interface
 * with black background and four buttons: Takeoff, Draw Sword, Rotate, Landing.
 * Each button displays different text when pressed.
 *
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 *
 */

#include "tuya_cloud_types.h"

#include "ui_display.h"

#include "font_awesome_symbols.h"
#include "lvgl.h"
#include "sword.c"

#include "tal_log.h"
#include "tkl_memory.h"

/***********************************************************
************************macro define************************
***********************************************************/
#define BUTTON_WIDTH 60
#define BUTTON_HEIGHT 50
#define BUTTON_MARGIN 5
#define STATUS_LABEL_HEIGHT 60

/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef struct {
    lv_obj_t *container;
    lv_obj_t *status_label;
    lv_obj_t *takeoff_btn;
    lv_obj_t *draw_sword_btn;
    lv_obj_t *rotate_btn;
    lv_obj_t *landing_btn;
    lv_obj_t *sword_img;
    lv_anim_t sword_anim;
} UI_CONTROL_T;

/***********************************************************
***********************variable define**********************
***********************************************************/
static UI_CONTROL_T sg_ui_control = {0};

/***********************************************************
***********************function define**********************
***********************************************************/

// Forward declarations
static void __sword_takeoff_animation(void);
static void __sword_landing_animation(void);

static void __takeoff_btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_CLICKED) {
        lv_label_set_text(sg_ui_control.status_label, "Taking off... Ready for flight!");
        __sword_takeoff_animation(); // Start sword upward animation
        PR_DEBUG("Takeoff button clicked");
    }
}

static void __draw_sword_btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_CLICKED) {
        lv_label_set_text(sg_ui_control.status_label, "Drawing sword... ");
        PR_DEBUG("Draw sword button clicked");
    }
}

static void __rotate_btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_CLICKED) {
        lv_label_set_text(sg_ui_control.status_label, "Rotating... Changing direction!");
        PR_DEBUG("Rotate button clicked");
    }
}

static void __landing_btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_CLICKED) {
        lv_label_set_text(sg_ui_control.status_label, "Landing... touch down!");
        __sword_landing_animation(); // Start sword downward animation
        PR_DEBUG("Landing button clicked");
    }
}

static void __sword_takeoff_animation(void)
{
    // Start sword upward movement animation
    lv_anim_init(&sg_ui_control.sword_anim);
    lv_anim_set_var(&sg_ui_control.sword_anim, sg_ui_control.sword_img);
    lv_anim_set_values(&sg_ui_control.sword_anim, 70, 0); // Move from 70px down to 0px (center)
    lv_anim_set_time(&sg_ui_control.sword_anim, 3000); // 3 seconds
    lv_anim_set_exec_cb(&sg_ui_control.sword_anim, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_path_cb(&sg_ui_control.sword_anim, lv_anim_path_ease_out);
    lv_anim_start(&sg_ui_control.sword_anim);
}

static void __sword_landing_animation(void)
{
    // Start sword downward movement animation
    lv_anim_init(&sg_ui_control.sword_anim);
    lv_anim_set_var(&sg_ui_control.sword_anim, sg_ui_control.sword_img);
    lv_anim_set_values(&sg_ui_control.sword_anim, 0, 70); // Move from 0px (center) to 70px down
    lv_anim_set_time(&sg_ui_control.sword_anim, 3000); // 3 seconds
    lv_anim_set_exec_cb(&sg_ui_control.sword_anim, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_path_cb(&sg_ui_control.sword_anim, lv_anim_path_ease_in);
    lv_anim_start(&sg_ui_control.sword_anim);
}


/**
 * @brief Initialize the control UI interface
 *
 * @param ui_font Font configuration for the UI
 * @return int 0 on success, -1 on failure
 */
int ui_control_init(UI_FONT_T *ui_font)
{
    if (ui_font == NULL) {
        PR_ERR("ui_font is NULL");
        return -1;
    }

    // Set screen background to black first
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), 0);
    lv_obj_set_style_bg_opa(lv_scr_act(), LV_OPA_COVER, 0);
    
    // Create main container
    sg_ui_control.container = lv_obj_create(lv_scr_act());
    lv_obj_set_size(sg_ui_control.container, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_pos(sg_ui_control.container, 0, 0);
    lv_obj_set_style_bg_color(sg_ui_control.container, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(sg_ui_control.container, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(sg_ui_control.container, 0, 0);
    lv_obj_set_style_pad_all(sg_ui_control.container, 0, 0);
    lv_obj_set_style_pad_left(sg_ui_control.container, 0, 0);
    lv_obj_set_style_pad_right(sg_ui_control.container, 0, 0);
    lv_obj_set_style_pad_top(sg_ui_control.container, 0, 0);
    lv_obj_set_style_pad_bottom(sg_ui_control.container, 0, 0);
    lv_obj_set_style_radius(sg_ui_control.container, 0, 0);
    lv_obj_clear_flag(sg_ui_control.container, LV_OBJ_FLAG_SCROLLABLE);

    // Create status label at the top
    sg_ui_control.status_label = lv_label_create(sg_ui_control.container);
    lv_label_set_text(sg_ui_control.status_label, "Control Panel Ready");
    lv_obj_set_style_text_color(sg_ui_control.status_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(sg_ui_control.status_label, ui_font->text, 0);
    lv_obj_align(sg_ui_control.status_label, LV_ALIGN_TOP_MID, 0, 20);
    lv_obj_set_width(sg_ui_control.status_label, LV_HOR_RES - 20);

    // Calculate button positions - 4 buttons in a row at the bottom
    int screen_width = LV_HOR_RES;
    int screen_height = LV_VER_RES;
    int button_y = screen_height - BUTTON_HEIGHT - 5; // 5px from bottom
    
    // Calculate dynamic button width to fit screen with 5px margins
    int available_width = screen_width - 10; // 5px margin on each side
    int total_margin_width = BUTTON_MARGIN * 3; // 3 margins between 4 buttons
    int dynamic_button_width = (available_width - total_margin_width) / 4;
    
    // Use calculated width for better distribution
    int final_button_width = dynamic_button_width;
    int total_button_width = (final_button_width * 4) + (BUTTON_MARGIN * 3);
    int start_x = (screen_width - total_button_width) / 2;
    
    // Create Takeoff button
    sg_ui_control.takeoff_btn = lv_btn_create(sg_ui_control.container);
    lv_obj_set_size(sg_ui_control.takeoff_btn, final_button_width, BUTTON_HEIGHT);
    lv_obj_set_pos(sg_ui_control.takeoff_btn, start_x, button_y);
    lv_obj_set_style_bg_color(sg_ui_control.takeoff_btn, lv_color_hex(0x2E7D32), 0);
    lv_obj_set_style_bg_color(sg_ui_control.takeoff_btn, lv_color_hex(0x1B5E20), LV_STATE_PRESSED);
    
    lv_obj_t *takeoff_label = lv_label_create(sg_ui_control.takeoff_btn);
    lv_label_set_text(takeoff_label, "Up");
    lv_obj_set_style_text_color(takeoff_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(takeoff_label, ui_font->text, 0);
    lv_obj_center(takeoff_label);
    lv_obj_add_event_cb(sg_ui_control.takeoff_btn, __takeoff_btn_event_cb, LV_EVENT_ALL, NULL);

    // Create Draw Sword button
    sg_ui_control.draw_sword_btn = lv_btn_create(sg_ui_control.container);
    lv_obj_set_size(sg_ui_control.draw_sword_btn, final_button_width, BUTTON_HEIGHT);
    lv_obj_set_pos(sg_ui_control.draw_sword_btn, start_x + final_button_width + BUTTON_MARGIN, button_y);
    lv_obj_set_style_bg_color(sg_ui_control.draw_sword_btn, lv_color_hex(0xD32F2F), 0);
    lv_obj_set_style_bg_color(sg_ui_control.draw_sword_btn, lv_color_hex(0xB71C1C), LV_STATE_PRESSED);
    
    lv_obj_t *draw_sword_label = lv_label_create(sg_ui_control.draw_sword_btn);
    lv_label_set_text(draw_sword_label, "Sword");
    lv_obj_set_style_text_color(draw_sword_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(draw_sword_label, ui_font->text, 0);
    lv_obj_center(draw_sword_label);
    lv_obj_add_event_cb(sg_ui_control.draw_sword_btn, __draw_sword_btn_event_cb, LV_EVENT_ALL, NULL);

    // Create Rotate button
    sg_ui_control.rotate_btn = lv_btn_create(sg_ui_control.container);
    lv_obj_set_size(sg_ui_control.rotate_btn, final_button_width, BUTTON_HEIGHT);
    lv_obj_set_pos(sg_ui_control.rotate_btn, start_x + (final_button_width + BUTTON_MARGIN) * 2, button_y);
    lv_obj_set_style_bg_color(sg_ui_control.rotate_btn, lv_color_hex(0x1976D2), 0);
    lv_obj_set_style_bg_color(sg_ui_control.rotate_btn, lv_color_hex(0x0D47A1), LV_STATE_PRESSED);
    
    lv_obj_t *rotate_label = lv_label_create(sg_ui_control.rotate_btn);
    lv_label_set_text(rotate_label, "Rotate");
    lv_obj_set_style_text_color(rotate_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(rotate_label, ui_font->text, 0);
    lv_obj_center(rotate_label);
    lv_obj_add_event_cb(sg_ui_control.rotate_btn, __rotate_btn_event_cb, LV_EVENT_ALL, NULL);

    // Create Landing button
    sg_ui_control.landing_btn = lv_btn_create(sg_ui_control.container);
    lv_obj_set_size(sg_ui_control.landing_btn, final_button_width, BUTTON_HEIGHT);
    lv_obj_set_pos(sg_ui_control.landing_btn, start_x + (final_button_width + BUTTON_MARGIN) * 3, button_y);
    lv_obj_set_style_bg_color(sg_ui_control.landing_btn, lv_color_hex(0xFF8F00), 0);
    lv_obj_set_style_bg_color(sg_ui_control.landing_btn, lv_color_hex(0xE65100), LV_STATE_PRESSED);
    
    lv_obj_t *landing_label = lv_label_create(sg_ui_control.landing_btn);
    lv_label_set_text(landing_label, "Landing");
    lv_obj_set_style_text_color(landing_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(landing_label, ui_font->text, 0);
    lv_obj_center(landing_label);
    lv_obj_add_event_cb(sg_ui_control.landing_btn, __landing_btn_event_cb, LV_EVENT_ALL, NULL);

    // Create sword image in the center of the screen, moved down 70 pixels
    sg_ui_control.sword_img = lv_img_create(sg_ui_control.container);
    lv_img_set_src(sg_ui_control.sword_img, &sword);
    lv_obj_align(sg_ui_control.sword_img, LV_ALIGN_CENTER, 0, 70);

    PR_DEBUG("Control UI initialized successfully");
    return 0;
}

/**
 * @brief Clean up the control UI
 */
void ui_control_cleanup(void)
{
    if (sg_ui_control.container) {
        lv_obj_del(sg_ui_control.container);
        memset(&sg_ui_control, 0, sizeof(UI_CONTROL_T));
    }
}
