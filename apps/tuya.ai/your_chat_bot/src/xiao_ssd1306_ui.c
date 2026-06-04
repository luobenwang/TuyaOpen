/**
 * @file xiao_ssd1306_ui.c
 * @brief Landscape minimal UI for XIAO SSD1306 128x64 OLED.
 * @version 4.8
 * @date 2026-06-02
 * @copyright Copyright (c) Tuya Inc.
 */

#include "xiao_ssd1306_ui.h"

#if defined(ENABLE_COMP_AI_DISPLAY) && (ENABLE_COMP_AI_DISPLAY == 1) && \
    defined(ENABLE_AI_CHAT_CUSTOM_UI) && (ENABLE_AI_CHAT_CUSTOM_UI == 1)

#include <stdint.h>
#include <string.h>

#include "tal_api.h"
#include "tal_time_service.h"
#include "lvgl.h"
#include "lv_vendor.h"
#include "ai_ui_manage.h"
#include "ai_ui_icon_font.h"
#include "lang_config.h"

/* ---------------------------------------------------------------------------
 * Layout — landscape 128 x 64
 * --------------------------------------------------------------------------- */
#define XIAO_UI_W                   (128)
#define XIAO_UI_H                   (64)

#define XIAO_UI_FONT_SMALL_H        (14)
#define XIAO_UI_BOTTOM_Y            (XIAO_UI_H - XIAO_UI_FONT_SMALL_H - 2)

#define XIAO_UI_STATUS_H            (14)
#define XIAO_UI_STATUS_W            (56)
#define XIAO_UI_STATUS_X            (2)
#define XIAO_UI_STATUS_Y            (1)

#define XIAO_UI_CHAT_PAD            (2)
#define XIAO_UI_CHAT_BODY_Y         (XIAO_UI_STATUS_Y + XIAO_UI_STATUS_H)
#define XIAO_UI_CHAT_BODY_W         (XIAO_UI_W - (XIAO_UI_CHAT_PAD * 2))
#define XIAO_UI_CHAT_BODY_H         (XIAO_UI_H - XIAO_UI_CHAT_BODY_Y - XIAO_UI_CHAT_PAD)
#define XIAO_UI_SCROLL_SPEED        (40)

#define XIAO_UI_CLOCK_TICK_MS       (1000)
#define XIAO_UI_CHAT_HOLD_MS        (8000)
#define XIAO_UI_AI_TEXT_MAX         (256)
#define XIAO_UI_STATUS_TEXT_MAX     (16)

#define XIAO_UI_BRAND_TEXT          "TUYA.AI"

/* ---------------------------------------------------------------------------
 * Type definitions
 * --------------------------------------------------------------------------- */
typedef struct {
    lv_obj_t *network_label;
    lv_obj_t *time_label;
    lv_obj_t *date_label;
    lv_obj_t *brand_label;
    lv_obj_t *clock_status_label;
    lv_obj_t *chat_panel;
    lv_obj_t *chat_status_label;
    lv_obj_t *chat_label;
    lv_timer_t *clock_timer;
    lv_timer_t *chat_hold_timer;
    char ai_text_buf[XIAO_UI_AI_TEXT_MAX];
    char status_buf[XIAO_UI_STATUS_TEXT_MAX];
    bool is_streaming;
    bool in_chat;
} XIAO_SSD1306_UI_T;

/* ---------------------------------------------------------------------------
 * File scope variables
 * --------------------------------------------------------------------------- */
static XIAO_SSD1306_UI_T s_ui;
static const lv_font_t *s_font_text = NULL;
static const lv_font_t *s_font_icon = NULL;

void tuya_app_gui_feed_watchdog(void);

/* ---------------------------------------------------------------------------
 * Style helpers — SSD1306 1-bit (RGB565 mono uses luma threshold in esp_lvgl_port)
 * --------------------------------------------------------------------------- */
/**
 * @brief Background color (black).
 * @return lv_color_t
 */
static lv_color_t __ui_color_off(void)
{
    return lv_color_black();
}

/**
 * @brief Foreground color (white).
 * @return lv_color_t
 */
static lv_color_t __ui_color_on(void)
{
    return lv_color_white();
}

/**
 * @brief Disable LVGL theme on OLED display.
 * @return none
 */
static void __ui_mono_display_setup(void)
{
    lv_display_t *disp = lv_display_get_default();

    if (disp == NULL) {
        PR_ERR("OLED display not ready");
        return;
    }
    lv_display_set_theme(disp, NULL);
    PR_NOTICE("OLED color_format=%d (0x12=RGB565 mono)", (int)lv_display_get_color_format(disp));
}

/**
 * @brief Reset screen: black background, no theme blocks.
 * @param[in] screen active screen object
 * @return none
 */
static void __ui_prepare_screen(lv_obj_t *screen)
{
    if (screen == NULL) {
        return;
    }
    lv_obj_remove_style_all(screen);
    lv_obj_set_size(screen, XIAO_UI_W, XIAO_UI_H);
    lv_obj_set_style_bg_color(screen, __ui_color_off(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_text_color(screen, __ui_color_on(), LV_PART_MAIN);
    lv_obj_set_style_border_width(screen, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(screen, 0, LV_PART_MAIN);
    lv_obj_set_style_clip_corner(screen, true, LV_PART_MAIN);
}

/**
 * @brief Style label: transparent background, mono-safe white text.
 * @param[in] label label object
 * @param[in] font font pointer or NULL
 * @return none
 */
static void __ui_label_mono(lv_obj_t *label, const lv_font_t *font)
{
    if (label == NULL) {
        return;
    }
    lv_obj_remove_style_all(label);
    lv_obj_set_style_bg_opa(label, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(label, 0, LV_PART_MAIN);
    lv_obj_set_style_outline_width(label, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(label, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(label, 0, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, __ui_color_on(), LV_PART_MAIN);
    lv_obj_set_style_text_opa(label, LV_OPA_COVER, LV_PART_MAIN);
    if (font != NULL) {
        lv_obj_set_style_text_font(label, font, LV_PART_MAIN);
    }
    lv_obj_remove_flag(label, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
}

/**
 * @brief Style panel: opaque black, clipped to screen.
 * @param[in] panel panel object
 * @return none
 */
static void __ui_panel_mono(lv_obj_t *panel)
{
    if (panel == NULL) {
        return;
    }
    lv_obj_remove_style_all(panel);
    lv_obj_set_style_bg_color(panel, __ui_color_off(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(panel, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(panel, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(panel, 0, LV_PART_MAIN);
    lv_obj_remove_flag(panel, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
}

/**
 * @brief Load fonts from AI UI module.
 * @return none
 */
static void __ui_font_init(void)
{
    s_font_text = ai_ui_get_text_font();
    s_font_icon = ai_ui_get_icon_font();
}

/**
 * @brief Map long status string to short top-left text (fits 56px).
 * @param[in] status full status from AI stack
 * @param[out] out output buffer
 * @param[in] out_sz output size
 * @return none
 */
static void __ui_status_to_short(const char *status, char *out, size_t out_sz)
{
    if (out == NULL || out_sz == 0) {
        return;
    }
    out[0] = '\0';
    if (status == NULL || status[0] == '\0') {
        snprintf(out, out_sz, "对话");
        return;
    }
    if (strcmp(status, STANDBY) == 0) {
        snprintf(out, out_sz, "待命");
    } else if (strcmp(status, LISTENING) == 0) {
        snprintf(out, out_sz, "聆听");
    } else if (strcmp(status, THINKING) == 0) {
        snprintf(out, out_sz, "思考");
    } else if (strcmp(status, SPEAKING) == 0) {
        snprintf(out, out_sz, "说话");
    } else if (strcmp(status, UPLOADING) == 0) {
        snprintf(out, out_sz, "上传");
    } else if (strcmp(status, PROVISIONING) == 0 || strcmp(status, ENTERING_WIFI_CONFIG_MODE) == 0) {
        snprintf(out, out_sz, "配网");
    } else if (strcmp(status, INITIALIZING) == 0) {
        snprintf(out, out_sz, "初始化");
    } else if (strcmp(status, CONNECT_SERVER) == 0) {
        snprintf(out, out_sz, "连接");
    } else {
        snprintf(out, out_sz, "%.6s", status);
    }
}

/**
 * @brief Refresh top-left status on clock screen and chat overlay.
 * @return none
 */
static void __ui_status_refresh(void)
{
    char short_txt[XIAO_UI_STATUS_TEXT_MAX];

    __ui_status_to_short(s_ui.status_buf, short_txt, sizeof(short_txt));
    if (s_ui.clock_status_label != NULL) {
        lv_label_set_text(s_ui.clock_status_label, short_txt);
    }
    if (s_ui.chat_status_label != NULL) {
        lv_label_set_text(s_ui.chat_status_label, short_txt);
    }
}

/**
 * @brief Store AI mode status string.
 * @param[in] status status from AI stack
 * @return none
 */
static void __ui_status_store(const char *status)
{
    if (status == NULL) {
        return;
    }
    snprintf(s_ui.status_buf, sizeof(s_ui.status_buf), "%s", status);
}

/**
 * @brief Append streaming AI text chunk.
 * @param[in] chunk UTF-8 fragment
 * @return none
 */
static void __ui_append_chat_text(const char *chunk)
{
    size_t cur = 0;
    size_t add = 0;

    if (chunk == NULL || s_ui.chat_label == NULL || chunk[0] == '\0') {
        return;
    }

    cur = strlen(s_ui.ai_text_buf);
    add = strlen(chunk);
    if (cur + add >= sizeof(s_ui.ai_text_buf)) {
        size_t drop = cur / 2;

        if (drop > 0) {
            memmove(s_ui.ai_text_buf, s_ui.ai_text_buf + drop, cur - drop + 1);
            cur = strlen(s_ui.ai_text_buf);
        } else {
            s_ui.ai_text_buf[0] = '\0';
            cur = 0;
        }
    }
    snprintf(s_ui.ai_text_buf + cur, sizeof(s_ui.ai_text_buf) - cur, "%s", chunk);
    lv_label_set_text(s_ui.chat_label, s_ui.ai_text_buf);
}

/**
 * @brief Show clock layer, hide full-screen chat overlay.
 * @return none
 */
static void __ui_show_clock(void)
{
    s_ui.in_chat = false;
    if (s_ui.chat_panel != NULL) {
        lv_obj_add_flag(s_ui.chat_panel, LV_OBJ_FLAG_HIDDEN);
    }
    if (s_ui.network_label != NULL) {
        lv_obj_clear_flag(s_ui.network_label, LV_OBJ_FLAG_HIDDEN);
    }
    if (s_ui.time_label != NULL) {
        lv_obj_clear_flag(s_ui.time_label, LV_OBJ_FLAG_HIDDEN);
    }
    if (s_ui.date_label != NULL) {
        lv_obj_clear_flag(s_ui.date_label, LV_OBJ_FLAG_HIDDEN);
    }
    if (s_ui.brand_label != NULL) {
        lv_obj_clear_flag(s_ui.brand_label, LV_OBJ_FLAG_HIDDEN);
    }
}

/**
 * @brief Cover clock UI with full-screen chat overlay.
 * @return none
 */
static void __ui_show_chat(void)
{
    s_ui.in_chat = true;
    if (s_ui.network_label != NULL) {
        lv_obj_add_flag(s_ui.network_label, LV_OBJ_FLAG_HIDDEN);
    }
    if (s_ui.time_label != NULL) {
        lv_obj_add_flag(s_ui.time_label, LV_OBJ_FLAG_HIDDEN);
    }
    if (s_ui.date_label != NULL) {
        lv_obj_add_flag(s_ui.date_label, LV_OBJ_FLAG_HIDDEN);
    }
    if (s_ui.brand_label != NULL) {
        lv_obj_add_flag(s_ui.brand_label, LV_OBJ_FLAG_HIDDEN);
    }
    if (s_ui.chat_panel != NULL) {
        lv_obj_clear_flag(s_ui.chat_panel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(s_ui.chat_panel);
    }
    __ui_status_refresh();
}

/**
 * @brief Refresh clock and date labels.
 * @return none
 */
static void __ui_clock_refresh(void)
{
    char date[16] = {0};
    char time[8] = {0};

    if (s_ui.time_label == NULL) {
        return;
    }

    if (tal_time_check_time_sync() != OPRT_OK) {
        lv_label_set_text(s_ui.time_label, "--:--");
        if (s_ui.date_label != NULL) {
            lv_label_set_text(s_ui.date_label, "--/--");
        }
        return;
    }

    POSIX_TM_S tm = {0};
    tal_time_get_local_time_custom(0, &tm);
    snprintf(time, sizeof(time), "%02d:%02d", tm.tm_hour, tm.tm_min);
    snprintf(date, sizeof(date), "%02d/%02d", tm.tm_mon + 1, tm.tm_mday);

    lv_label_set_text(s_ui.time_label, time);
    if (s_ui.date_label != NULL) {
        lv_label_set_text(s_ui.date_label, date);
    }
}

/**
 * @brief Clock timer callback.
 * @param[in] timer unused
 * @return none
 */
static void __ui_clock_timer_cb(lv_timer_t *timer)
{
    (void)timer;
    lv_vendor_disp_lock();
    if (!s_ui.in_chat && !s_ui.is_streaming) {
        __ui_clock_refresh();
    }
    lv_vendor_disp_unlock();
}

/**
 * @brief Return to clock after chat hold timeout.
 * @param[in] timer unused
 * @return none
 */
static void __ui_chat_hold_cb(lv_timer_t *timer)
{
    (void)timer;
    lv_vendor_disp_lock();
    s_ui.is_streaming = false;
    s_ui.ai_text_buf[0] = '\0';
    __ui_show_clock();
    __ui_clock_refresh();
    lv_vendor_disp_unlock();
}

/**
 * @brief Time sync event callback.
 * @param[in] data unused
 * @return 0
 */
static int __ui_time_sync_cb(void *data)
{
    (void)data;
    lv_vendor_disp_lock();
    if (!s_ui.in_chat) {
        __ui_clock_refresh();
    }
    lv_vendor_disp_unlock();
    return 0;
}

/**
 * @brief Create a fresh root screen (drop LVGL default themed screen).
 * @return new screen object
 */
static lv_obj_t *__ui_create_root_screen(void)
{
    lv_obj_t *prev = lv_screen_active();
    lv_obj_t *screen = lv_obj_create(NULL);

    __ui_prepare_screen(screen);
    lv_screen_load(screen);
    if (prev != NULL && prev != screen) {
        lv_obj_delete(prev);
    }
    return screen;
}

/**
 * @brief Setup full-body chat label: circular scroll within panel bounds.
 * @param[in] label chat label
 * @return none
 */
static void __ui_chat_scroll_setup(lv_obj_t *label)
{
    if (label == NULL) {
        return;
    }
    lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    lv_obj_set_size(label, XIAO_UI_CHAT_BODY_W, XIAO_UI_CHAT_BODY_H);
    lv_obj_set_style_anim_duration(label, lv_anim_speed_clamped(XIAO_UI_SCROLL_SPEED, 200, 60000), LV_PART_MAIN);
    lv_obj_set_pos(label, XIAO_UI_CHAT_PAD, XIAO_UI_CHAT_BODY_Y);
}

/**
 * @brief Setup small status label (top-left, clipped).
 * @param[in] label status label
 * @return none
 */
static void __ui_status_label_setup(lv_obj_t *label)
{
    if (label == NULL) {
        return;
    }
    lv_label_set_long_mode(label, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    lv_obj_set_size(label, XIAO_UI_STATUS_W, XIAO_UI_STATUS_H);
    lv_obj_set_pos(label, XIAO_UI_STATUS_X, XIAO_UI_STATUS_Y);
}

/**
 * @brief Create a mono-safe label on parent.
 * @param[in] parent parent object
 * @param[in] text initial text
 * @param[in] font font pointer
 * @return label object
 */
static lv_obj_t *__ui_add_label(lv_obj_t *parent, const char *text, const lv_font_t *font)
{
    lv_obj_t *lbl = lv_label_create(parent);

    lv_label_set_text(lbl, text);
    __ui_label_mono(lbl, font);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_CLIP);
    return lbl;
}

/**
 * @brief Build landscape layout.
 * @return none
 */
static void __ui_build_layout(void)
{
    lv_obj_t *screen = __ui_create_root_screen();

    s_ui.clock_status_label = lv_label_create(screen);
    lv_label_set_text(s_ui.clock_status_label, "");
    __ui_label_mono(s_ui.clock_status_label, s_font_text);
    __ui_status_label_setup(s_ui.clock_status_label);

    s_ui.network_label = __ui_add_label(screen, "", s_font_icon);
    lv_obj_align(s_ui.network_label, LV_ALIGN_TOP_RIGHT, -2, 2);

    s_ui.time_label = __ui_add_label(screen, "--:--", s_font_text);
    lv_obj_align(s_ui.time_label, LV_ALIGN_CENTER, 0, -4);

    s_ui.date_label = __ui_add_label(screen, "", s_font_text);
    lv_obj_align(s_ui.date_label, LV_ALIGN_BOTTOM_LEFT, 4, -2);

    s_ui.brand_label = __ui_add_label(screen, XIAO_UI_BRAND_TEXT, s_font_text);
    lv_obj_align(s_ui.brand_label, LV_ALIGN_BOTTOM_RIGHT, -4, -2);
    tuya_app_gui_feed_watchdog();

    s_ui.chat_panel = lv_obj_create(screen);
    __ui_panel_mono(s_ui.chat_panel);
    lv_obj_set_size(s_ui.chat_panel, XIAO_UI_W, XIAO_UI_H);
    lv_obj_align(s_ui.chat_panel, LV_ALIGN_TOP_LEFT, 0, 0);

    s_ui.chat_status_label = lv_label_create(s_ui.chat_panel);
    lv_label_set_text(s_ui.chat_status_label, "");
    __ui_label_mono(s_ui.chat_status_label, s_font_text);
    __ui_status_label_setup(s_ui.chat_status_label);

    s_ui.chat_label = lv_label_create(s_ui.chat_panel);
    lv_label_set_text(s_ui.chat_label, "");
    __ui_label_mono(s_ui.chat_label, s_font_text);
    __ui_chat_scroll_setup(s_ui.chat_label);

    lv_obj_add_flag(s_ui.chat_panel, LV_OBJ_FLAG_HIDDEN);
    snprintf(s_ui.status_buf, sizeof(s_ui.status_buf), "%s", STANDBY);

    s_ui.clock_timer = lv_timer_create(__ui_clock_timer_cb, XIAO_UI_CLOCK_TICK_MS, NULL);
    s_ui.chat_hold_timer = lv_timer_create(__ui_chat_hold_cb, XIAO_UI_CHAT_HOLD_MS, NULL);
    lv_timer_pause(s_ui.chat_hold_timer);

    tal_event_subscribe("app.time.sync", "xiao_ssd1306_ui", __ui_time_sync_cb, SUBSCRIBE_TYPE_NORMAL);
    __ui_show_clock();
    __ui_status_refresh();
    __ui_clock_refresh();
    tuya_app_gui_feed_watchdog();
}

/**
 * @brief Initialize LVGL display port.
 * @return none
 */
static void __lvgl_init(void)
{
    lv_vendor_init(DISPLAY_NAME);
}

/**
 * @brief UI module init entry.
 * @return OPRT_OK on success
 */
static OPERATE_RET __ui_init(void)
{
    memset(&s_ui, 0, sizeof(s_ui));

    __lvgl_init();
    lv_vendor_disp_lock();
    __ui_mono_display_setup();
    __ui_font_init();
    __ui_build_layout();
    lv_obj_invalidate(lv_screen_active());
    __ui_status_refresh();
    __ui_clock_refresh();
    lv_vendor_disp_unlock();

    return OPRT_OK;
}

/**
 * @brief Enter chat view with full-screen scrolling text.
 * @param[in] text message body or NULL
 * @param[in] hold_ms hold before clock, 0 to pause timer
 * @return none
 */
static void __ui_enter_chat(const char *text, uint32_t hold_ms)
{
    __ui_show_chat();
    if (text != NULL) {
        snprintf(s_ui.ai_text_buf, sizeof(s_ui.ai_text_buf), "%s", text);
    } else {
        s_ui.ai_text_buf[0] = '\0';
    }
    lv_label_set_text(s_ui.chat_label, s_ui.ai_text_buf);
    if (s_ui.chat_hold_timer != NULL) {
        if (hold_ms > 0) {
            lv_timer_set_period(s_ui.chat_hold_timer, hold_ms);
            lv_timer_reset(s_ui.chat_hold_timer);
            lv_timer_resume(s_ui.chat_hold_timer);
        } else {
            lv_timer_pause(s_ui.chat_hold_timer);
        }
    }
}

/**
 * @brief Display user ASR text.
 * @param[in] text user message
 * @return none
 */
static void __ui_set_user_msg(char *text)
{
    if (text == NULL) {
        return;
    }

    lv_vendor_disp_lock();
    s_ui.is_streaming = false;
    __ui_enter_chat(text, XIAO_UI_CHAT_HOLD_MS);
    lv_vendor_disp_unlock();
}

/**
 * @brief Display full AI message.
 * @param[in] text AI message
 * @return none
 */
static void __ui_set_ai_msg(char *text)
{
    if (text == NULL) {
        return;
    }

    lv_vendor_disp_lock();
    s_ui.is_streaming = false;
    __ui_enter_chat(text, XIAO_UI_CHAT_HOLD_MS);
    lv_vendor_disp_unlock();
}

/**
 * @brief Start AI text stream.
 * @return none
 */
static void __ui_set_ai_msg_stream_start(void)
{
    lv_vendor_disp_lock();
    s_ui.is_streaming = true;
    s_ui.ai_text_buf[0] = '\0';
    __ui_show_chat();
    lv_label_set_text(s_ui.chat_label, "");
    if (s_ui.chat_hold_timer != NULL) {
        lv_timer_pause(s_ui.chat_hold_timer);
    }
    lv_vendor_disp_unlock();
}

/**
 * @brief Append AI stream chunk.
 * @param[in] text text chunk
 * @return none
 */
static void __ui_set_ai_msg_stream_data(char *text)
{
    if (text == NULL || text[0] == '\0') {
        return;
    }

    lv_vendor_disp_lock();
    if (!s_ui.is_streaming) {
        s_ui.is_streaming = true;
        s_ui.ai_text_buf[0] = '\0';
        __ui_show_chat();
        lv_label_set_text(s_ui.chat_label, "");
        if (s_ui.chat_hold_timer != NULL) {
            lv_timer_pause(s_ui.chat_hold_timer);
        }
    }
    __ui_append_chat_text(text);
    lv_vendor_disp_unlock();
}

/**
 * @brief End AI text stream.
 * @return none
 */
static void __ui_set_ai_msg_stream_end(void)
{
    lv_vendor_disp_lock();
    s_ui.is_streaming = false;
    if (s_ui.chat_hold_timer != NULL) {
        lv_timer_set_period(s_ui.chat_hold_timer, XIAO_UI_CHAT_HOLD_MS);
        lv_timer_reset(s_ui.chat_hold_timer);
        lv_timer_resume(s_ui.chat_hold_timer);
    }
    lv_vendor_disp_unlock();
}

/**
 * @brief Ignore emotion icons (default OLED UI draws emoji here).
 * @param[in] emotion emotion name
 * @return none
 */
static void __ui_set_emotion(char *emotion)
{
    (void)emotion;
}

/**
 * @brief Update AI mode status (top-left on clock and chat screens).
 * @param[in] status status string
 * @return none
 */
static void __ui_set_status(char *status)
{
    if (status == NULL) {
        return;
    }

    lv_vendor_disp_lock();
    __ui_status_store(status);
    __ui_status_refresh();
    if (strcmp(status, STANDBY) == 0 && !s_ui.is_streaming && s_ui.ai_text_buf[0] == '\0') {
        if (s_ui.chat_hold_timer != NULL) {
            lv_timer_pause(s_ui.chat_hold_timer);
        }
        __ui_show_clock();
        __ui_clock_refresh();
    }
    lv_vendor_disp_unlock();
}

/**
 * @brief Update WiFi icon (top-right, clock view only).
 * @param[in] wifi_status WiFi level
 * @return none
 */
static void __ui_set_network(AI_UI_WIFI_STATUS_E wifi_status)
{
    char *icon = ai_ui_get_wifi_icon(wifi_status);

    if (s_ui.network_label == NULL || icon == NULL) {
        return;
    }

    lv_vendor_disp_lock();
    lv_label_set_text(s_ui.network_label, icon);
    lv_vendor_disp_unlock();
}

/**
 * @brief Yield CPU during LVGL init.
 * @return none
 */
void tuya_app_gui_feed_watchdog(void)
{
    tal_system_sleep(1);
}

/**
 * @brief Register UI callbacks with AI UI manager.
 * @return OPRT_OK on success
 */
OPERATE_RET xiao_ssd1306_ui_register(void)
{
    AI_UI_INTFS_T intfs;

    memset(&intfs, 0, sizeof(AI_UI_INTFS_T));
    intfs.disp_init = __ui_init;
    intfs.disp_user_msg = __ui_set_user_msg;
    intfs.disp_ai_msg = __ui_set_ai_msg;
    intfs.disp_ai_msg_stream_start = __ui_set_ai_msg_stream_start;
    intfs.disp_ai_msg_stream_data = __ui_set_ai_msg_stream_data;
    intfs.disp_ai_msg_stream_end = __ui_set_ai_msg_stream_end;
    intfs.disp_system_msg = __ui_set_ai_msg;
    intfs.disp_emotion = __ui_set_emotion;
    intfs.disp_ai_mode_state = __ui_set_status;
    intfs.disp_wifi_state = __ui_set_network;

    return ai_ui_register(&intfs);
}

#endif /* ENABLE_COMP_AI_DISPLAY && ENABLE_AI_CHAT_CUSTOM_UI */
