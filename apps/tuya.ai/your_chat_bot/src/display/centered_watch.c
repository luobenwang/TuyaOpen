/**
 * @file centered_watch.c
 * Properly centered analog watch with correct center point calculations
 */

/*********************
 *      INCLUDES
 *********************/
#include "vintage_watch_app.h"
#include "tal_time_service.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

/*********************
 *      DEFINES
 *********************/
#define PI 3.14159265359f
#define DEG_TO_RAD (PI / 180.0f)
#define RAD_TO_DEG (180.0f / PI)

/* Screen center calculations */
#define SCREEN_CENTER_X (WATCH_SCREEN_WIDTH / 2)   /* 120 */
#define SCREEN_CENTER_Y (WATCH_SCREEN_HEIGHT / 2)  /* 120 */
#define SCREEN_CENTER (SCREEN_CENTER_X)            /* 120 */

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    lv_obj_t *screen;
    lv_obj_t *viewport;
    lv_obj_t *watch_face;
    lv_obj_t *hour_hand;
    lv_obj_t *minute_hand;
    lv_obj_t *second_hand;
    lv_obj_t *center_pin;
    lv_timer_t *time_timer;
    struct tm current_time;
} centered_watch_t;

/**********************
 *  STATIC VARIABLES
 **********************/
static centered_watch_t g_watch;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void create_root(void);
static void create_watch_face(void);
static void create_watch_marks(void);
static void create_watch_hands(void);
static void update_hands(void);
static void on_time_tick(lv_timer_t *timer);

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void lv_demo_vintage_watch(void)
{
    memset(&g_watch, 0, sizeof(g_watch));
    
    /* Get current time */
    time_t now = time(NULL);
    g_watch.current_time = *localtime(&now);
    
    printf("Centered Watch: Screen %dx%d, Center at (%d,%d)\n", 
           WATCH_SCREEN_WIDTH, WATCH_SCREEN_HEIGHT, SCREEN_CENTER_X, SCREEN_CENTER_Y);
    printf("Starting at %02d:%02d:%02d\n", 
           g_watch.current_time.tm_hour, 
           g_watch.current_time.tm_min, 
           g_watch.current_time.tm_sec);
    
    /* Create UI components */
    create_root();
    create_watch_face();
    create_watch_marks();
    create_watch_hands();
    update_hands();
    
    /* Start time update timer */
    g_watch.time_timer = lv_timer_create(on_time_tick, 1000, NULL);
}

// 全局同步状态
static bool g_time_synced = false;

void vintage_watch_update_time(void)
{
    // 如果时间未同步，等待同步
    if (!g_time_synced) {
        printf("Watch waiting for time sync...\n");
        return;
    }
    
    // 使用Tuya时间服务获取本地时间（包含时区）
    POSIX_TM_S tm;
    OPERATE_RET ret = tal_time_get_local_time_custom(0, &tm);
    if (ret != OPRT_OK) {
        printf("Watch local time get failed: %d\n", ret);
        return;
    }
    
    // 转换为标准tm结构
    g_watch.current_time.tm_sec = tm.tm_sec;
    g_watch.current_time.tm_min = tm.tm_min;
    g_watch.current_time.tm_hour = tm.tm_hour;
    g_watch.current_time.tm_mday = tm.tm_mday;
    g_watch.current_time.tm_mon = tm.tm_mon;
    g_watch.current_time.tm_year = tm.tm_year;
    g_watch.current_time.tm_wday = tm.tm_wday;
    
    printf("Watch Update: %02d:%02d:%02d\n", 
           g_watch.current_time.tm_hour, 
           g_watch.current_time.tm_min, 
           g_watch.current_time.tm_sec);
    
    update_hands();
}

void vintage_watch_set_sync_time(time_t sync_time)
{
    printf("Watch sync time set: %ld\n", sync_time);
    // 设置同步状态，让时钟开始转动
    g_time_synced = true;
}

void vintage_watch_set_mode(watch_mode_t mode) { (void)mode; }
void vintage_watch_set_style(watch_style_t style) { (void)style; }
void vintage_watch_show_date(void) {}
void vintage_watch_hide_date(void) {}
void vintage_watch_toggle_seconds(void) {}
void vintage_watch_set_face_color(uint32_t color) { (void)color; }
void vintage_watch_set_hands_color(uint32_t color) { (void)color; }
void vintage_watch_set_text_color(uint32_t color) { (void)color; }
void vintage_watch_start_smooth_hands(void) {}
void vintage_watch_stop_smooth_hands(void) {}
void vintage_watch_set_animation_speed(float speed) { (void)speed; }

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void create_root(void)
{
    g_watch.screen = lv_obj_create(NULL);
    lv_obj_set_size(g_watch.screen, WATCH_SCREEN_WIDTH, WATCH_SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(g_watch.screen, lv_color_hex(VINTAGE_BG_COLOR), 0);
    lv_obj_set_style_bg_opa(g_watch.screen, LV_OPA_COVER, 0);
    
    /* Circular viewport */
    g_watch.viewport = lv_obj_create(g_watch.screen);
    lv_obj_set_size(g_watch.viewport, WATCH_SCREEN_WIDTH, WATCH_SCREEN_HEIGHT);
    lv_obj_center(g_watch.viewport);
    lv_obj_set_style_radius(g_watch.viewport, WATCH_RADIUS, 0);
    lv_obj_set_style_clip_corner(g_watch.viewport, true, 0);
    lv_obj_set_style_border_width(g_watch.viewport, 0, 0);
    lv_obj_set_style_bg_color(g_watch.viewport, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(g_watch.viewport, LV_OPA_COVER, 0);
    
    lv_screen_load(g_watch.screen);
}

static void create_watch_face(void)
{
    /* Main watch face with vintage styling */
    g_watch.watch_face = lv_obj_create(g_watch.viewport);
    lv_obj_set_size(g_watch.watch_face, WATCH_SCREEN_WIDTH - 10, WATCH_SCREEN_HEIGHT - 10);
    lv_obj_center(g_watch.watch_face);
    lv_obj_set_style_radius(g_watch.watch_face, (WATCH_SCREEN_WIDTH - 10) / 2, 0);
    lv_obj_set_style_bg_color(g_watch.watch_face, lv_color_hex(VINTAGE_FACE_COLOR), 0);
    lv_obj_set_style_bg_opa(g_watch.watch_face, LV_OPA_COVER, 0);
    
    /* 复古边框效果 */
    lv_obj_set_style_border_width(g_watch.watch_face, 4, 0);
    lv_obj_set_style_border_color(g_watch.watch_face, lv_color_hex(VINTAGE_GOLD_COLOR), 0);
    
    /* 添加阴影效果 */
    lv_obj_set_style_shadow_width(g_watch.watch_face, 8, 0);
    lv_obj_set_style_shadow_color(g_watch.watch_face, lv_color_hex(VINTAGE_SHADOW_COLOR), 0);
    lv_obj_set_style_shadow_ofs_x(g_watch.watch_face, 2, 0);
    lv_obj_set_style_shadow_ofs_y(g_watch.watch_face, 2, 0);
    
    lv_obj_clear_flag(g_watch.watch_face, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(g_watch.watch_face, LV_OBJ_FLAG_SCROLLABLE);
}

static void create_watch_marks(void)
{
    /* 创建12个小时刻度 - 所有刻度大小一样 */
    for (int i = 0; i < 12; i++) {
        float angle = i * 30.0f - 90.0f;  /* 从12点开始，顺时针 */
        float rad = angle * DEG_TO_RAD;
        
        /* 计算刻度位置 */
        int mark_radius = WATCH_RADIUS - 20;
        int mark_x = SCREEN_CENTER_X + (int)(mark_radius * cos(rad));
        int mark_y = SCREEN_CENTER_Y + (int)(mark_radius * sin(rad));
        
        /* 创建刻度矩形 - 统一大小 */
        lv_obj_t *mark = lv_obj_create(g_watch.screen);
        lv_obj_remove_style_all(mark);
        lv_obj_set_size(mark, 2, 10);  /* 统一：2px宽，10px长 */
        lv_obj_set_pos(mark, mark_x - 1, mark_y - 5);
        lv_obj_set_style_bg_color(mark, lv_color_hex(VINTAGE_GOLD_COLOR), 0);
        lv_obj_set_style_bg_opa(mark, LV_OPA_COVER, 0);
        lv_obj_set_style_radius(mark, 1, 0);
        lv_obj_clear_flag(mark, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(mark, LV_OBJ_FLAG_SCROLLABLE);
        
        /* 设置刻度旋转 */
        lv_obj_set_style_transform_angle(mark, (int16_t)(angle * 10), 0);
        lv_obj_set_style_transform_pivot_x(mark, 1, 0);
        lv_obj_set_style_transform_pivot_y(mark, 5, 0);
    }
    
    /* 创建12个小时数字 */
    const char* hour_labels[] = {"12", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11"};
    for (int i = 0; i < 12; i++) {
        float angle = i * 30.0f - 90.0f;  /* 从12点开始，顺时针 */
        float rad = angle * DEG_TO_RAD;
        
        /* 计算数字位置 */
        int label_radius = WATCH_RADIUS - 35;
        int label_x = SCREEN_CENTER_X + (int)(label_radius * cos(rad));
        int label_y = SCREEN_CENTER_Y + (int)(label_radius * sin(rad));
        
        /* 创建数字标签 */
        lv_obj_t *label = lv_label_create(g_watch.screen);
        lv_label_set_text(label, hour_labels[i]);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
        lv_obj_set_style_text_color(label, lv_color_hex(VINTAGE_TEXT_COLOR), 0);
        lv_obj_set_style_text_opa(label, LV_OPA_COVER, 0);
        lv_obj_set_pos(label, label_x - 8, label_y - 7);
        lv_obj_clear_flag(label, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(label, LV_OBJ_FLAG_SCROLLABLE);
    }
    
    /* 创建60个分钟刻度 */
    for (int i = 0; i < 60; i++) {
        if (i % 5 == 0) continue;  /* 跳过小时刻度位置 */
        
        float angle = i * 6.0f - 90.0f;  /* 每分钟6度 */
        float rad = angle * DEG_TO_RAD;
        
        /* 计算刻度位置 */
        int mark_radius = WATCH_RADIUS - 15;
        int mark_x = SCREEN_CENTER_X + (int)(mark_radius * cos(rad));
        int mark_y = SCREEN_CENTER_Y + (int)(mark_radius * sin(rad));
        
        /* 创建小刻度矩形 */
        lv_obj_t *mark = lv_obj_create(g_watch.screen);
        lv_obj_remove_style_all(mark);
        lv_obj_set_size(mark, 1, 3);
        lv_obj_set_pos(mark, mark_x, mark_y - 1);
        lv_obj_set_style_bg_color(mark, lv_color_hex(VINTAGE_GOLD_COLOR), 0);
        lv_obj_set_style_bg_opa(mark, LV_OPA_COVER, 0);
        lv_obj_clear_flag(mark, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(mark, LV_OBJ_FLAG_SCROLLABLE);
        
        /* 设置刻度旋转 */
        lv_obj_set_style_transform_angle(mark, (int16_t)(angle * 10), 0);
        lv_obj_set_style_transform_pivot_x(mark, 0, 0);
        lv_obj_set_style_transform_pivot_y(mark, 1, 0);
    }
    
    /* 添加典雅的装饰环 - 多层精致效果 */
    lv_obj_t *decorative_ring = lv_obj_create(g_watch.screen);
    lv_obj_remove_style_all(decorative_ring);
    lv_obj_set_size(decorative_ring, WATCH_RADIUS * 2 - 10, WATCH_RADIUS * 2 - 10);
    lv_obj_center(decorative_ring);
    lv_obj_set_style_radius(decorative_ring, WATCH_RADIUS - 5, 0);
    lv_obj_set_style_border_width(decorative_ring, 2, 0);
    lv_obj_set_style_border_color(decorative_ring, lv_color_hex(VINTAGE_GOLD_COLOR), 0);
    lv_obj_set_style_bg_opa(decorative_ring, LV_OPA_TRANSP, 0);
    
    /* 添加中层装饰环 */
    lv_obj_t *middle_ring = lv_obj_create(g_watch.screen);
    lv_obj_remove_style_all(middle_ring);
    lv_obj_set_size(middle_ring, WATCH_RADIUS * 2 - 16, WATCH_RADIUS * 2 - 16);
    lv_obj_center(middle_ring);
    lv_obj_set_style_radius(middle_ring, WATCH_RADIUS - 8, 0);
    lv_obj_set_style_border_width(middle_ring, 1, 0);
    lv_obj_set_style_border_color(middle_ring, lv_color_hex(VINTAGE_PLATINUM_COLOR), 0);
    lv_obj_set_style_bg_opa(middle_ring, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(middle_ring, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(middle_ring, LV_OBJ_FLAG_SCROLLABLE);
    
    /* 添加内层装饰环 */
    lv_obj_t *inner_ring = lv_obj_create(g_watch.screen);
    lv_obj_remove_style_all(inner_ring);
    lv_obj_set_size(inner_ring, WATCH_RADIUS * 2 - 22, WATCH_RADIUS * 2 - 22);
    lv_obj_center(inner_ring);
    lv_obj_set_style_radius(inner_ring, WATCH_RADIUS - 11, 0);
    lv_obj_set_style_border_width(inner_ring, 1, 0);
    lv_obj_set_style_border_color(inner_ring, lv_color_hex(VINTAGE_DARK_GOLD), 0);
    lv_obj_set_style_bg_opa(inner_ring, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(inner_ring, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(inner_ring, LV_OBJ_FLAG_SCROLLABLE);
    
    lv_obj_clear_flag(decorative_ring, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(decorative_ring, LV_OBJ_FLAG_SCROLLABLE);
    
    printf("Created 12 hour marks, numbers, 60 minute marks, and decorative ring\n");
}

static void create_watch_hands(void)
{
    /* Hour hand - positioned at screen center, extending upward to 12 o'clock */
    g_watch.hour_hand = lv_obj_create(g_watch.screen);
    lv_obj_remove_style_all(g_watch.hour_hand);
    lv_obj_set_size(g_watch.hour_hand, 6, 40);  /* 6px thick, 40px long - vertical */
    lv_obj_set_style_bg_color(g_watch.hour_hand, lv_color_hex(VINTAGE_GOLD_COLOR), 0);
    lv_obj_set_style_bg_opa(g_watch.hour_hand, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(g_watch.hour_hand, 3, 0);
    /* Position at screen center, extending upward to 12 o'clock */
    lv_obj_set_pos(g_watch.hour_hand, SCREEN_CENTER_X - 3, SCREEN_CENTER_Y - 40);
    lv_obj_clear_flag(g_watch.hour_hand, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(g_watch.hour_hand, LV_OBJ_FLAG_SCROLLABLE);
    
    /* Minute hand - positioned at screen center, extending upward to 12 o'clock */
    g_watch.minute_hand = lv_obj_create(g_watch.screen);
    lv_obj_remove_style_all(g_watch.minute_hand);
    lv_obj_set_size(g_watch.minute_hand, 4, 55);  /* 4px thick, 55px long - vertical */
    lv_obj_set_style_bg_color(g_watch.minute_hand, lv_color_hex(VINTAGE_GOLD_COLOR), 0);
    lv_obj_set_style_bg_opa(g_watch.minute_hand, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(g_watch.minute_hand, 2, 0);
    /* Position at screen center, extending upward to 12 o'clock */
    lv_obj_set_pos(g_watch.minute_hand, SCREEN_CENTER_X - 2, SCREEN_CENTER_Y - 55);
    lv_obj_clear_flag(g_watch.minute_hand, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(g_watch.minute_hand, LV_OBJ_FLAG_SCROLLABLE);
    
    /* Second hand - positioned at screen center, extending upward to 12 o'clock */
    g_watch.second_hand = lv_obj_create(g_watch.screen);
    lv_obj_remove_style_all(g_watch.second_hand);
    lv_obj_set_size(g_watch.second_hand, 2, 65);  /* 2px thick, 65px long - vertical */
    lv_obj_set_style_bg_color(g_watch.second_hand, lv_color_hex(0xFF0000), 0);
    lv_obj_set_style_bg_opa(g_watch.second_hand, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(g_watch.second_hand, 1, 0);
    /* Position at screen center, extending upward to 12 o'clock */
    lv_obj_set_pos(g_watch.second_hand, SCREEN_CENTER_X - 1, SCREEN_CENTER_Y - 65);
    lv_obj_clear_flag(g_watch.second_hand, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(g_watch.second_hand, LV_OBJ_FLAG_SCROLLABLE);
    
    printf("Created watch hands: Hour at (%d,%d), Minute at (%d,%d), Second at (%d,%d)\n",
           SCREEN_CENTER_X, SCREEN_CENTER_Y - 3,
           SCREEN_CENTER_X, SCREEN_CENTER_Y - 2,
           SCREEN_CENTER_X, SCREEN_CENTER_Y - 1);
    
    /* 典雅的中心装饰 - 多层设计 */
    g_watch.center_pin = lv_obj_create(g_watch.screen);
    lv_obj_remove_style_all(g_watch.center_pin);
    lv_obj_set_size(g_watch.center_pin, 10, 10);
    lv_obj_set_pos(g_watch.center_pin, SCREEN_CENTER_X - 5, SCREEN_CENTER_Y - 5);
    lv_obj_set_style_bg_color(g_watch.center_pin, lv_color_hex(VINTAGE_GOLD_COLOR), 0);
    lv_obj_set_style_radius(g_watch.center_pin, 5, 0);
    lv_obj_set_style_bg_opa(g_watch.center_pin, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(g_watch.center_pin, 1, 0);
    lv_obj_set_style_border_color(g_watch.center_pin, lv_color_hex(VINTAGE_DARK_GOLD), 0);
    lv_obj_clear_flag(g_watch.center_pin, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(g_watch.center_pin, LV_OBJ_FLAG_SCROLLABLE);
    
    /* 内层中心装饰 */
    lv_obj_t *inner_center = lv_obj_create(g_watch.screen);
    lv_obj_remove_style_all(inner_center);
    lv_obj_set_size(inner_center, 6, 6);
    lv_obj_set_pos(inner_center, SCREEN_CENTER_X - 3, SCREEN_CENTER_Y - 3);
    lv_obj_set_style_bg_color(inner_center, lv_color_hex(VINTAGE_PLATINUM_COLOR), 0);
    lv_obj_set_style_radius(inner_center, 3, 0);
    lv_obj_set_style_bg_opa(inner_center, LV_OPA_COVER, 0);
    lv_obj_clear_flag(inner_center, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(inner_center, LV_OBJ_FLAG_SCROLLABLE);
}

static void update_hands(void)
{
    /* Calculate angles in degrees - 0° at 12 o'clock position */
    float hour_angle = ((g_watch.current_time.tm_hour % 12) * 30.0f + 
                       (g_watch.current_time.tm_min / 60.0f) * 30.0f);
    float minute_angle = g_watch.current_time.tm_min * 6.0f;
    float second_angle = g_watch.current_time.tm_sec * 6.0f;
    
    printf("Time: %02d:%02d:%02d -> Hour=%.1f°, Minute=%.1f°, Second=%.1f°\n", 
           g_watch.current_time.tm_hour, g_watch.current_time.tm_min, g_watch.current_time.tm_sec,
           hour_angle, minute_angle, second_angle);
    
    printf("Angle calculation: Hour=%d*30+%.2f*30=%.1f°, Min=%d*6=%.1f°, Sec=%d*6=%.1f°\n",
           (g_watch.current_time.tm_hour % 12), (g_watch.current_time.tm_min / 60.0f), hour_angle,
           g_watch.current_time.tm_min, minute_angle,
           g_watch.current_time.tm_sec, second_angle);
    
    // 测试：强制设置12:00:00来验证角度计算
    if (g_watch.current_time.tm_hour == 23 && g_watch.current_time.tm_min == 59 && g_watch.current_time.tm_sec == 59) {
        printf("Testing 12:00:00 angles: Hour=0°, Minute=0°, Second=0°\n");
        hour_angle = 0.0f;
        minute_angle = 0.0f; 
        second_angle = 0.0f;
    }
    
    /* Update hour hand rotation - pivot at watch center */
    lv_obj_set_style_transform_angle(g_watch.hour_hand, (int16_t)(hour_angle * 10), 0);
    lv_obj_set_style_transform_pivot_x(g_watch.hour_hand, 3, 0);  /* Pivot at watch center */
    lv_obj_set_style_transform_pivot_y(g_watch.hour_hand, 40, 0);  /* Pivot at watch center */
    
    /* Update minute hand rotation - pivot at watch center */
    lv_obj_set_style_transform_angle(g_watch.minute_hand, (int16_t)(minute_angle * 10), 0);
    lv_obj_set_style_transform_pivot_x(g_watch.minute_hand, 2, 0);  /* Pivot at watch center */
    lv_obj_set_style_transform_pivot_y(g_watch.minute_hand, 55, 0);  /* Pivot at watch center */
    
    /* Update second hand rotation - pivot at watch center */
    lv_obj_set_style_transform_angle(g_watch.second_hand, (int16_t)(second_angle * 10), 0);
    lv_obj_set_style_transform_pivot_x(g_watch.second_hand, 1, 0);  /* Pivot at watch center */
    lv_obj_set_style_transform_pivot_y(g_watch.second_hand, 65, 0);  /* Pivot at watch center */
    
    /* Force refresh of the objects */
    lv_obj_invalidate(g_watch.hour_hand);
    lv_obj_invalidate(g_watch.minute_hand);
    lv_obj_invalidate(g_watch.second_hand);
}

static void on_time_tick(lv_timer_t *timer)
{
    (void)timer;
    printf("Timer tick - updating watch hands\n");
    vintage_watch_update_time();
}
