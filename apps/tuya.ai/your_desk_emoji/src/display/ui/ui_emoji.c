#include "tuya_cloud_types.h"

#if defined(ENABLE_GUI_EMOJI) && (ENABLE_GUI_EMOJI == 1)

#include "tal_system.h"
#include "ui_display.h"
#include "lvgl.h"

typedef enum {
    UI_EYE_NEUTRAL = 0,
    UI_EYE_HAPPY,
    UI_EYE_SAD,
    UI_EYE_ANGRY,
    UI_EYE_SURPRISE,
    UI_EYE_SLEEP,
    UI_EYE_WAKEUP,
    UI_EYE_BLINK,
    UI_EYE_LEFT,
    UI_EYE_RIGHT,
    UI_EYE_CENTER,
    UI_EYE_EMOTION_MAX,
} UI_EYE_EMOTION_E;

typedef struct {
    char emo_text[32];
    UI_EYE_EMOTION_E emo_value;
} UI_EYE_EMOTION_LIST_T;

#define EYE_W                  80
#define EYE_H                  80
#define EYE_SPACE              20
#define EYE_RADIUS             10

static lv_obj_t *eye_left      = NULL;
static lv_obj_t *eye_right     = NULL;
static lv_obj_t *eye_parent    = NULL;
static lv_obj_t *eye_container = NULL;

static int eye_center_x        = 0;
static int eye_center_y        = 0;

static int eye_left_x          = 0;
static int eye_left_y          = 0;
static int eye_right_x         = 0;
static int eye_right_y         = 0;

static UI_EYE_EMOTION_LIST_T s_eye_emo_list[UI_EYE_EMOTION_MAX] = {
    {"NEUTRAL", UI_EYE_NEUTRAL},   {"SAD", UI_EYE_SAD},
    {"ANGRY", UI_EYE_ANGRY},       {"SURPRISE", UI_EYE_SURPRISE},
    {"BLINK", UI_EYE_BLINK},       {"HAPPY", UI_EYE_HAPPY},
    {"LEFT", UI_EYE_LEFT},         {"RIGHT", UI_EYE_RIGHT},
    {"SLEEP", UI_EYE_SLEEP},       {"WAKEUP", UI_EYE_WAKEUP},
    {"CENTER", UI_EYE_CENTER}
};

static void lv_eye_init(lv_obj_t *parent)
{
    eye_parent = parent;
    int scr_w = lv_obj_get_width(parent);
    int scr_h = lv_obj_get_height(parent);

    eye_center_x = scr_w / 2;
    eye_center_y = scr_h / 2;

    // Create a container
    eye_container = lv_obj_create(parent);
    lv_obj_set_size(eye_container, scr_w, scr_h);
    lv_obj_set_style_bg_color(eye_container, lv_color_black(), 0); // black background
    lv_obj_set_style_border_width(eye_container, 0, 0);
    lv_obj_clear_flag(eye_container, LV_OBJ_FLAG_SCROLLABLE);

    // Left eye
    eye_left = lv_obj_create(eye_container);
    lv_obj_set_size(eye_left, EYE_W, EYE_H);
    lv_obj_set_style_radius(eye_left, EYE_RADIUS, 0);
    lv_obj_set_style_bg_color(eye_left, lv_color_white(), 0);
    lv_obj_set_style_border_width(eye_left, 2, 0);
    lv_obj_set_style_border_color(eye_left, lv_color_make(120, 120, 140), 0); // 机器人风格边框
    eye_left_x = eye_center_x - EYE_W - EYE_SPACE/2;
    eye_left_y = eye_center_y - EYE_H/2;
    lv_obj_set_pos(eye_left, eye_left_x, eye_left_y);

    // Right eye
    eye_right = lv_obj_create(eye_container);
    lv_obj_set_size(eye_right, EYE_W, EYE_H);
    lv_obj_set_style_radius(eye_right, EYE_RADIUS, 0);
    lv_obj_set_style_bg_color(eye_right, lv_color_white(), 0);
    lv_obj_set_style_border_width(eye_right, 2, 0);
    lv_obj_set_style_border_color(eye_right, lv_color_make(120, 120, 140), 0);
    eye_right_x = eye_center_x + EYE_SPACE/2;
    eye_right_y = eye_center_y - EYE_H/2;
    lv_obj_set_pos(eye_right, eye_right_x, eye_right_y);
}

static void lv_eye_deinit(void)
{
    if (eye_left) {
        lv_obj_del(eye_left);
    }
    if (eye_right) {
        lv_obj_del(eye_right);
    }
    if (eye_container) {
        lv_obj_del(eye_container);
    }
    eye_left = eye_right = eye_container = NULL;
}

static void lv_eye_center(void)
{
    if (!eye_left || !eye_right) {
        return;
    }
    lv_obj_set_size(eye_left, EYE_W, EYE_H);
    lv_obj_set_size(eye_right, EYE_W, EYE_H);
    eye_left_x = eye_center_x - EYE_W - EYE_SPACE/2;
    eye_left_y = eye_center_y - EYE_H/2;
    eye_right_x = eye_center_x + EYE_SPACE/2;
    eye_right_y = eye_center_y - EYE_H/2;
    lv_obj_set_pos(eye_left, eye_left_x, eye_left_y);
    lv_obj_set_pos(eye_right, eye_right_x, eye_right_y);
}

static void lv_eye_blink(void)
{
    if (!eye_left || !eye_right) {
        return;
    }
    for (int i = 0; i < 3; i++) {
        lv_obj_set_height(eye_left, EYE_H/3);
        lv_obj_set_height(eye_right, EYE_H/3);
        lv_timer_handler();
        lv_tick_inc(80);
        tal_system_sleep(80);
    }
    for (int i = 0; i < 3; i++) {
        lv_obj_set_height(eye_left, EYE_H);
        lv_obj_set_height(eye_right, EYE_H);
        lv_timer_handler();
        lv_tick_inc(80);
        tal_system_sleep(80);
    }
}

static void lv_eye_sleep(void)
{
    if (!eye_left || !eye_right) {
        return;
    }
    lv_obj_set_height(eye_left, 4);
    lv_obj_set_height(eye_right, 4);
    lv_timer_handler();
}

static void lv_eye_wakeup(void)
{
    if (!eye_left || !eye_right) {
        return;
    }
    for (int h = 4; h <= EYE_H; h += 4) {
        lv_obj_set_height(eye_left, h);
        lv_obj_set_height(eye_right, h);
        lv_timer_handler();
        lv_tick_inc(30);
        tal_system_sleep(30);
    }
}

static void lv_eye_left(void)
{
    if (!eye_left || !eye_right) {
        return;
    }
    int move_steps = 6;
    int move_px = 4;
    for (int i = 0; i < move_steps; i++) {
        eye_left_x -= move_px;
        eye_right_x -= move_px;
        lv_obj_set_pos(eye_left, eye_left_x, eye_left_y);
        lv_obj_set_pos(eye_right, eye_right_x, eye_right_y);
        lv_timer_handler();
        lv_tick_inc(15);
        tal_system_sleep(15);
    }
    // 停留一会
    lv_tick_inc(120);
    tal_system_sleep(120);
    for (int i = 0; i < move_steps; i++) {
        eye_left_x += move_px;
        eye_right_x += move_px;
        lv_obj_set_pos(eye_left, eye_left_x, eye_left_y);
        lv_obj_set_pos(eye_right, eye_right_x, eye_right_y);
        lv_timer_handler();
        lv_tick_inc(15);
        tal_system_sleep(15);
    }
    lv_eye_center();
}

static void lv_eye_right(void)
{
    if (!eye_left || !eye_right) {
        return;
    }
    int move_steps = 6;
    int move_px = 4;
    for (int i = 0; i < move_steps; i++) {
        eye_left_x += move_px;
        eye_right_x += move_px;
        lv_obj_set_pos(eye_left, eye_left_x, eye_left_y);
        lv_obj_set_pos(eye_right, eye_right_x, eye_right_y);
        lv_timer_handler();
        lv_tick_inc(15);
        tal_system_sleep(15);
    }
    // 停留一会
    lv_tick_inc(120);
    tal_system_sleep(120);
    for (int i = 0; i < move_steps; i++) {
        eye_left_x -= move_px;
        eye_right_x -= move_px;
        lv_obj_set_pos(eye_left, eye_left_x, eye_left_y);
        lv_obj_set_pos(eye_right, eye_right_x, eye_right_y);
        lv_timer_handler();
        lv_tick_inc(15);
        tal_system_sleep(15);
    }
    lv_eye_center();
}

static void lv_eye_happy(void)
{
    lv_eye_center();
    lv_obj_set_height(eye_left, EYE_H-10);
    lv_obj_set_height(eye_right, EYE_H-10);
    lv_timer_handler();
}

static void lv_eye_sad(void)
{
    lv_eye_center();
    lv_obj_set_height(eye_left, EYE_H-20);
    lv_obj_set_height(eye_right, EYE_H-20);
    lv_timer_handler();
}

static void lv_eye_anger(void)
{
    lv_eye_center();
    lv_obj_set_width(eye_left, EYE_W-10);
    lv_obj_set_width(eye_right, EYE_W-10);
    lv_timer_handler();
}

static void lv_eye_surprise(void)
{
    lv_eye_center();
    for (int w = EYE_W; w > 10; w -= 4) {
        lv_obj_set_size(eye_left, w, w);
        lv_obj_set_size(eye_right, w, w);
        lv_timer_handler();
        lv_tick_inc(30);
        tal_system_sleep(30);
    }
    lv_eye_center();
}

static void lv_eye_expression(UI_EYE_EMOTION_E expr)
{
    switch (expr) {
        case UI_EYE_NEUTRAL:
        case UI_EYE_WAKEUP:
            lv_eye_wakeup();
            break;
        case UI_EYE_HAPPY:
            lv_eye_happy();
            break;
        case UI_EYE_SAD:
            lv_eye_sad();
            break;
        case UI_EYE_ANGRY:
            lv_eye_anger();
            break;
        case UI_EYE_SURPRISE:
            lv_eye_surprise();
            break;
        case UI_EYE_SLEEP:
            lv_eye_sleep();
            break;
        case UI_EYE_BLINK:
            lv_eye_blink();
            break;
        case UI_EYE_LEFT:
            lv_eye_left();
            break;
        case UI_EYE_RIGHT:
            lv_eye_right();
            break;
        case UI_EYE_CENTER:
            lv_eye_center();
            break;
        default:
            break;
    }
}

// --- UI Init ---
int ui_init(UI_FONT_T *ui_font)
{
    lv_obj_t *screen = lv_screen_active();

    lv_eye_init(screen);

    return 0;
}

void ui_set_user_msg(const char *text)
{
    return;
}

void ui_set_assistant_msg(const char *text)
{
    return;
}

void ui_set_system_msg(const char *text)
{
    return;
}

void ui_set_emotion(const char *emotion)
{
    UI_EYE_EMOTION_E emo_val = UI_EYE_EMOTION_MAX;
    for (int i = 0; i < UI_EYE_EMOTION_MAX; i++) {
        if (strcmp(emotion, s_eye_emo_list[i].emo_text) == 0) {
            emo_val = s_eye_emo_list[i].emo_value;
            lv_eye_expression(emo_val);
            break;
        }
    }
}

void ui_set_status(const char *status)
{
    return;
}

void ui_set_notification(const char *notification)
{
    return;
}

void ui_set_network(char *wifi_icon)
{
    return;
}

void ui_set_chat_mode(const char *chat_mode)
{
    return;
}

void ui_set_status_bar_pad(int32_t value)
{
    return;
}

#endif