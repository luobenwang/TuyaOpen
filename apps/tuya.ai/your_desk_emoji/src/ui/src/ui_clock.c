/**
 * @file ui_clock.c
 * @brief Cyber / neon dark clock (scheme B: purple-blue gradient, cyan time, HUD strip)
 * @version 0.3
 * @copyright Copyright (c) 2021-2026 Tuya Inc. All Rights Reserved.
 */
#include <time.h>
#include "tal_api.h"
#include "lvgl.h"
#include "ai_ui_manage.h"
#include "ai_ui_icon_font.h"
#include "font_awesome_symbols.h"
#include "app_ui.h"

LV_FONT_DECLARE(lv_font_montserrat_14);

/***********************************************************
************************macro define************************
***********************************************************/
#ifndef lv_obj_get_content_width
#define lv_obj_get_content_width  lv_obj_get_width
#define lv_obj_get_content_height lv_obj_get_height
#endif

#define WEATHER_CLOCK_UPDATE_INTERVAL_MS    1000    // 1 second update interval
#define WEATHER_ICON_NAME_NUM               8
/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef struct {
    lv_obj_t   *container;
    lv_obj_t   *top_bar;
    lv_obj_t   *time_label;
    lv_obj_t   *date_label;
    lv_obj_t   *temperature_label;
    lv_obj_t   *weather_icon_img;
} AI_UI_CLOCK_T;

typedef struct {
    const lv_img_dsc_t *img;
    char* name[WEATHER_ICON_NAME_NUM];
}UI_WEATHER_ICON_T;

/***********************************************************
***********************variable define**********************
***********************************************************/
LV_IMG_DECLARE(img_sun_120);
LV_IMG_DECLARE(img_cloudy_129);
LV_IMG_DECLARE(img_rain_112);
LV_IMG_DECLARE(img_small_rain_139);
LV_IMG_DECLARE(img_snow_105);
LV_IMG_DECLARE(img_thunder_110);
LV_IMG_DECLARE(img_thundershower_143);
LV_IMG_DECLARE(img_windy_114);

static const UI_WEATHER_ICON_T cWEATHER_ICONS[] = {
    {&img_sun_120,             {"120", "119", NULL,  NULL,  NULL,  NULL,  NULL,  NULL}},
    {&img_cloudy_129,          {"129", "142", "132", NULL,  NULL,  NULL,  NULL,  NULL}},
    {&img_rain_112,            {"112", "101", "107", "108", NULL,  NULL,  NULL,  NULL}},
    {&img_small_rain_139,      {"139", NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL}},
    {&img_snow_105,            {"105", "104", "115", "124", "126", NULL,  NULL,  NULL}},
    {&img_thunder_110,         {"110", "138", NULL,  NULL,  NULL,  NULL,  NULL,  NULL}},
    {&img_thundershower_143,   {"143", "102", NULL,  NULL,  NULL,  NULL,  NULL,  NULL}},
    {&img_windy_114,           {"114", NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL}},
};


static AI_UI_CLOCK_T sg_ui;
static AI_UI_FONT_LIST_T sg_font = {0};

/***********************************************************
***********************function define**********************
***********************************************************/
static void __ui_font_init(void)
{
    sg_font.text       = ai_ui_get_text_font();
    sg_font.icon       = ai_ui_get_icon_font();
    sg_font.emoji      = ai_ui_get_emo_font();
    sg_font.emoji_list = ai_ui_get_emo_list();
}

static const lv_image_dsc_t* __get_weather_img(char *name)
{
    if(NULL == name) {
        return NULL;
    }

    for (int i = 0; i < CNTSOF(cWEATHER_ICONS); i++) {
        for(int j = 0; j < WEATHER_ICON_NAME_NUM; j++) {
            if(cWEATHER_ICONS[i].name[j] == NULL) {
                continue;
            }

            if (0 == strcasecmp(cWEATHER_ICONS[i].name[j], name)) {
                return (lv_image_dsc_t *)cWEATHER_ICONS[i].img;
            }
        }
    }

    return cWEATHER_ICONS[0].img;
}


int __ui_clock_init(void)
{
    __ui_font_init();

    lv_obj_t *screen = lv_screen_active();

    /* Cyber night: purple top -> near-black blue bottom */
    lv_obj_set_style_bg_grad_dir(screen, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x1E0B42), 0);
    lv_obj_set_style_bg_grad_color(screen, lv_color_hex(0x040814), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

    sg_ui.container = lv_obj_create(screen);
    lv_obj_set_size(sg_ui.container, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_pad_all(sg_ui.container, 0, 0);
    lv_obj_set_style_border_width(sg_ui.container, 0, 0);
    lv_obj_set_style_radius(sg_ui.container, 0, 0);
    lv_obj_set_style_bg_opa(sg_ui.container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_shadow_width(sg_ui.container, 0, 0);
    lv_obj_add_flag(sg_ui.container, LV_OBJ_FLAG_OVERFLOW_VISIBLE);

    /* HUD strip: dark panel + neon cyan frame */
    sg_ui.top_bar = lv_obj_create(sg_ui.container);
    lv_obj_set_size(sg_ui.top_bar, LV_HOR_RES - 6, 24);
    lv_obj_align(sg_ui.top_bar, LV_ALIGN_TOP_MID, 0, 3);
    lv_obj_set_style_radius(sg_ui.top_bar, 8, 0);
    lv_obj_set_style_bg_color(sg_ui.top_bar, lv_color_hex(0x100820), 0);
    lv_obj_set_style_bg_opa(sg_ui.top_bar, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(sg_ui.top_bar, 1, 0);
    lv_obj_set_style_border_color(sg_ui.top_bar, lv_color_hex(0x00FFC6), 0);
    lv_obj_set_style_border_opa(sg_ui.top_bar, LV_OPA_80, 0);
    lv_obj_set_style_pad_left(sg_ui.top_bar, 8, 0);
    lv_obj_set_style_pad_right(sg_ui.top_bar, 6, 0);
    lv_obj_set_style_pad_ver(sg_ui.top_bar, 0, 0);
    lv_obj_set_scrollbar_mode(sg_ui.top_bar, LV_SCROLLBAR_MODE_OFF);

    sg_ui.date_label = lv_label_create(sg_ui.top_bar);
    lv_label_set_text(sg_ui.date_label, "--.--");
    lv_obj_set_style_text_font(sg_ui.date_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(sg_ui.date_label, lv_color_hex(0xE056FD), 0);
    lv_obj_set_style_text_opa(sg_ui.date_label, LV_OPA_COVER, 0);
    lv_obj_set_style_text_letter_space(sg_ui.date_label, 2, 0);
    lv_obj_set_style_text_align(sg_ui.date_label, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_align(sg_ui.date_label, LV_ALIGN_LEFT_MID, 0, 0);

    sg_ui.weather_icon_img = lv_image_create(sg_ui.top_bar);
    lv_obj_set_size(sg_ui.weather_icon_img, 18, 18);
    lv_obj_set_style_image_recolor(sg_ui.weather_icon_img, lv_color_hex(0x00FFF0), 0);
    lv_obj_set_style_image_recolor_opa(sg_ui.weather_icon_img, LV_OPA_50, 0);
    lv_obj_align(sg_ui.weather_icon_img, LV_ALIGN_RIGHT_MID, -2, 0);
    lv_obj_add_flag(sg_ui.weather_icon_img, LV_OBJ_FLAG_HIDDEN);

    sg_ui.temperature_label = lv_label_create(sg_ui.top_bar);
    lv_label_set_text(sg_ui.temperature_label, "--\xc2\xb0""C");
    lv_obj_set_style_text_font(sg_ui.temperature_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(sg_ui.temperature_label, lv_color_hex(0x7AF8FF), 0);
    lv_obj_set_style_text_align(sg_ui.temperature_label, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_style_text_letter_space(sg_ui.temperature_label, 1, 0);
    lv_obj_align_to(sg_ui.temperature_label, sg_ui.weather_icon_img, LV_ALIGN_OUT_LEFT_MID, -6, 0);
    lv_obj_add_flag(sg_ui.temperature_label, LV_OBJ_FLAG_HIDDEN);

    /* Electric cyan time + dark halo (outline) */
    sg_ui.time_label = lv_label_create(sg_ui.container);
    lv_label_set_text(sg_ui.time_label, "OFFLINE");
    lv_obj_set_style_text_font(sg_ui.time_label, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(sg_ui.time_label, lv_color_hex(0x00FFD5), 0);
    lv_obj_set_style_text_align(sg_ui.time_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_letter_space(sg_ui.time_label, 2, 0);
    lv_obj_set_style_outline_width(sg_ui.time_label, 2, 0);
    lv_obj_set_style_outline_color(sg_ui.time_label, lv_color_hex(0x240038), 0);
    lv_obj_set_style_outline_opa(sg_ui.time_label, LV_OPA_COVER, 0);
    lv_obj_set_style_outline_pad(sg_ui.time_label, 0, 0);
    lv_obj_set_width(sg_ui.time_label, LV_HOR_RES);
    lv_obj_align(sg_ui.time_label, LV_ALIGN_CENTER, 0, 7);

    lv_obj_clear_flag(sg_ui.container, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(sg_ui.container);

    PR_DEBUG("Cyber neon weather clock UI initialized");

    return 0;
}

void __ui_clock_show(void)
{
    /* Hide other UI elements first by moving weather clock to front */
    lv_obj_move_foreground(sg_ui.container);

    lv_obj_clear_flag(sg_ui.container, LV_OBJ_FLAG_HIDDEN);
}

void __ui_clock_hide(void)
{
    lv_obj_add_flag(sg_ui.container, LV_OBJ_FLAG_HIDDEN);
}

void __ui_clock_update_weather(int weather_code, int temperature)
{
    char weather_code_str[8] = {0};
    char temper_str[16] = {0};

    snprintf(weather_code_str, sizeof(weather_code_str), "%d", weather_code);
    snprintf(temper_str, sizeof(temper_str), "%d\xc2\xb0""C", temperature);

    const lv_image_dsc_t *icon_dsc = __get_weather_img(weather_code_str);
    if (icon_dsc != NULL) {
        lv_image_set_src(sg_ui.weather_icon_img, icon_dsc);
        lv_obj_set_style_image_recolor(sg_ui.weather_icon_img, lv_color_hex(0x00FFF0), 0);
        lv_obj_set_style_image_recolor_opa(sg_ui.weather_icon_img, LV_OPA_50, 0);
        lv_obj_clear_flag(sg_ui.weather_icon_img, LV_OBJ_FLAG_HIDDEN);
    }

    lv_label_set_text(sg_ui.temperature_label, temper_str);
    lv_obj_clear_flag(sg_ui.temperature_label, LV_OBJ_FLAG_HIDDEN);
}

void __ui_clock_update_time(POSIX_TM_S *curr_time)
{
    if(NULL == curr_time) {
        PR_ERR("time update info is NULL");
        return;
    } 

    char time_str[16];
    snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d",
                                         curr_time->tm_hour,\
                                         curr_time->tm_min, \
                                         curr_time->tm_sec);
    lv_label_set_text(sg_ui.time_label, time_str);

    char date_str[16];
    snprintf(date_str, sizeof(date_str), "%02d.%02d",
                                         curr_time->tm_mon + 1,
                                         curr_time->tm_mday);
    lv_label_set_text(sg_ui.date_label, date_str);
}