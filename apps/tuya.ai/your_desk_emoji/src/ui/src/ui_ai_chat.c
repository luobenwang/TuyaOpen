/**
 * @file ui_ai_chat.c
 * @brief Bottom single-line transient text for user and assistant (no prefix, no frame)
 * @version 1.2
 * @date 2026-05-11
 * @copyright Copyright (c) 2026 Tuya Inc. All Rights Reserved.
 */

#include <stddef.h>
#include <string.h>

#include "tal_api.h"
#include "lvgl.h"
#include "lv_vendor.h"
#include "ai_ui_icon_font.h"
#include "ui_ai_chat.h"

/* ---------------------------------------------------------------------------
 * Macros
 * --------------------------------------------------------------------------- */
#define UI_AI_CHAT_USER_MAX   192
#define UI_AI_CHAT_AI_MAX    640
/** Hold non-streaming assistant text before auto-clear (ms) */
#define UI_AI_CHAT_AI_HOLD_MS 2500

/* ---------------------------------------------------------------------------
 * File scope variables
 * --------------------------------------------------------------------------- */
static lv_obj_t   *s_label;
static lv_timer_t *s_ai_hold_timer;
static char        s_user[UI_AI_CHAT_USER_MAX];
static char        s_ai[UI_AI_CHAT_AI_MAX];
static bool        s_streaming;

/* ---------------------------------------------------------------------------
 * Forward declarations
 * --------------------------------------------------------------------------- */
static void __ai_hold_timer_cb(lv_timer_t *t);

/* ---------------------------------------------------------------------------
 * Function implementations
 * --------------------------------------------------------------------------- */
/**
 * @brief Stop the one-shot timer that clears non-streaming assistant text
 * @return none
 */
static void __cancel_ai_hold_timer(void)
{
    if (s_ai_hold_timer != NULL) {
        lv_timer_delete(s_ai_hold_timer);
        s_ai_hold_timer = NULL;
    }
}

/**
 * @brief Timer callback: clear assistant buffer after one-shot hold
 * @param[in] t timer instance
 * @return none
 */
static void __ai_hold_timer_cb(lv_timer_t *t)
{
    lv_vendor_disp_lock();
    lv_timer_delete(t);
    s_ai_hold_timer = NULL;
    s_ai[0] = '\0';
    if (s_label != NULL) {
        lv_label_set_text(s_label, "");
    }
    lv_vendor_disp_unlock();
}

/**
 * @brief Replace CR/LF with spaces so the label never wraps to a second line
 * @param[in,out] buf mutable UTF-8 string
 * @return none
 */
static void __strip_line_breaks(char *buf)
{
    char *p;

    if (buf == NULL) {
        return;
    }
    for (p = buf; *p != '\0'; p++) {
        if (*p == '\n' || *p == '\r') {
            *p = ' ';
        }
    }
}

/**
 * @brief Update label from buffers (plain text, no prefixes)
 * @return none
 */
static void __ui_ai_chat_refresh(void)
{
    if (s_label == NULL) {
        return;
    }

    if (s_streaming) {
        if (s_ai[0] != '\0') {
            lv_label_set_text(s_label, s_ai);
        } else {
            lv_label_set_text(s_label, "");
        }
        return;
    }

    if (s_ai[0] != '\0') {
        lv_label_set_text(s_label, s_ai);
        return;
    }

    if (s_user[0] != '\0') {
        lv_label_set_text(s_label, s_user);
        return;
    }

    lv_label_set_text(s_label, "");
}

/**
 * @brief Copy a C string into a fixed-size buffer, truncate, then flatten line breaks
 * @param[out] dst destination buffer
 * @param[in] dst_sz total size of dst including the terminating NUL
 * @param[in] src source C string (may be NULL)
 * @return none
 */
static void __copy_cstr(char *dst, size_t dst_sz, const char *src)
{
    if (dst == NULL || dst_sz == 0U) {
        return;
    }
    if (src == NULL) {
        dst[0] = '\0';
        return;
    }
    (void)snprintf(dst, dst_sz, "%s", src);
    __strip_line_breaks(dst);
}

/**
 * @brief Initialize bottom single-line label (no panel)
 * @return 0 on success, -1 if an LVGL object could not be created
 */
int __ui_ai_chat_init(void)
{
    lv_obj_t   *scr;
    lv_font_t  *font;
    lv_coord_t  line_h;

    scr = lv_screen_active();
    if (scr == NULL) {
        return -1;
    }

    s_label = lv_label_create(scr);
    if (s_label == NULL) {
        return -1;
    }

    font = ai_ui_get_text_font();
    if (font != NULL) {
        line_h = (lv_coord_t)lv_font_get_line_height(font);
        lv_obj_set_style_text_font(s_label, font, 0);
    } else {
        line_h = 16;
    }

    lv_obj_set_width(s_label, (lv_coord_t)(LV_HOR_RES - 8));
    lv_obj_set_height(s_label, line_h);
    lv_obj_align(s_label, LV_ALIGN_BOTTOM_MID, 0, -4);
    lv_label_set_long_mode(s_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_style_text_color(s_label, lv_color_hex(0xB8FFF5), 0);
    lv_obj_set_style_text_align(s_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_bg_opa(s_label, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(s_label, 0, 0);
    lv_obj_set_style_shadow_width(s_label, 0, 0);
    lv_obj_set_style_pad_all(s_label, 0, 0);
    lv_label_set_text(s_label, "");

    s_user[0]       = '\0';
    s_ai[0]         = '\0';
    s_streaming     = false;
    s_ai_hold_timer = NULL;

    lv_obj_move_foreground(s_label);

    return 0;
}

/**
 * @brief Show user text; clears assistant buffer
 * @param[in] string user UTF-8 text
 * @return none
 */
void __ui_ai_chat_set_user_msg(char *string)
{
    lv_vendor_disp_lock();
    __cancel_ai_hold_timer();
    __copy_cstr(s_user, sizeof(s_user), string);
    s_ai[0]     = '\0';
    s_streaming = false;
    __ui_ai_chat_refresh();
    lv_vendor_disp_unlock();
}

/**
 * @brief Show full assistant message (non-stream), then auto-clear
 * @param[in] string assistant UTF-8 text
 * @return none
 */
void __ui_ai_chat_set_ai_msg(char *string)
{
    lv_vendor_disp_lock();
    __cancel_ai_hold_timer();
    s_user[0]   = '\0';
    s_streaming = false;
    __copy_cstr(s_ai, sizeof(s_ai), string);
    __ui_ai_chat_refresh();

    s_ai_hold_timer = lv_timer_create(__ai_hold_timer_cb, UI_AI_CHAT_AI_HOLD_MS, NULL);
    if (s_ai_hold_timer != NULL) {
        lv_timer_set_repeat_count(s_ai_hold_timer, 1);
    }
    lv_vendor_disp_unlock();
}

/**
 * @brief Start streamed assistant reply; clears user text on screen
 * @return none
 */
void __ui_ai_chat_ai_stream_start(void)
{
    lv_vendor_disp_lock();
    __cancel_ai_hold_timer();
    s_user[0]   = '\0';
    s_ai[0]     = '\0';
    s_streaming = true;
    __ui_ai_chat_refresh();
    lv_vendor_disp_unlock();
}

/**
 * @brief Append streamed assistant UTF-8 chunk
 * @param[in] string UTF-8 chunk
 * @return none
 */
void __ui_ai_chat_ai_stream_data(char *string)
{
    size_t la;
    size_t rem;

    if (!s_streaming || string == NULL) {
        return;
    }

    lv_vendor_disp_lock();
    la = strlen(s_ai);
    if (la >= sizeof(s_ai) - 1U) {
        lv_vendor_disp_unlock();
        return;
    }
    rem = sizeof(s_ai) - la;
    (void)snprintf(s_ai + la, rem, "%s", string);
    __strip_line_breaks(s_ai);
    __ui_ai_chat_refresh();
    lv_vendor_disp_unlock();
}

/**
 * @brief End streamed assistant reply and clear display
 * @return none
 */
void __ui_ai_chat_ai_stream_end(void)
{
    lv_vendor_disp_lock();
    s_streaming = false;
    s_ai[0]     = '\0';
    __ui_ai_chat_refresh();
    lv_vendor_disp_unlock();
}
