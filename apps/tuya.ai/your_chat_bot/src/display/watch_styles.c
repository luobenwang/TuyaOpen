/**
 * @file watch_styles.c
 * @brief Implementation of different watch face styles
 * 
 * This file provides 4 different watch face styles:
 * 1. Anime Style - Bright, colorful with anime-inspired design
 * 2. Fashion Style - Elegant, modern with luxury elements
 * 3. Tech Style - Futuristic, digital with neon accents
 * 4. Forest Style - Natural, organic with earth tones
 */

#include "vintage_watch_app.h"
#include "ui_display.h"
#include "watch_styles.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

/*********************
 *      DEFINES
 *********************/
#define PI 3.14159265359f
#define DEG_TO_RAD (PI / 180.0f)

/* Anime Style Colors */
#define ANIME_BG_COLOR 0xFF69B4        /* Hot pink background */
#define ANIME_FACE_COLOR 0xFFE4E1      /* Misty rose face */
#define ANIME_ACCENT_COLOR 0xFF1493    /* Deep pink accent */
#define ANIME_HAND_COLOR 0xFF6347      /* Tomato hand color */
#define ANIME_TEXT_COLOR 0x8B008B      /* Dark magenta text */

/* Fashion Style Colors */
#define FASHION_BG_COLOR 0x2F2F2F      /* Dark gray background */
#define FASHION_FACE_COLOR 0xF5F5DC    /* Beige face */
#define FASHION_ACCENT_COLOR 0xDAA520  /* Goldenrod accent */
#define FASHION_HAND_COLOR 0x8B4513    /* Saddle brown hands */
#define FASHION_TEXT_COLOR 0x2F2F2F    /* Dark gray text */

/* Tech Style Colors */
#define TECH_BG_COLOR 0x000000         /* Black background */
#define TECH_FACE_COLOR 0x001122       /* Dark blue face */
#define TECH_ACCENT_COLOR 0x00FFFF     /* Cyan accent */
#define TECH_HAND_COLOR 0x00FF00       /* Green hands */
#define TECH_TEXT_COLOR 0x00FFFF       /* Cyan text */

/* Forest Style Colors */
#define FOREST_BG_COLOR 0x2E8B57       /* Sea green background */
#define FOREST_FACE_COLOR 0xF0FFF0     /* Honeydew face */
#define FOREST_ACCENT_COLOR 0x228B22   /* Forest green accent */
#define FOREST_HAND_COLOR 0x8B4513     /* Saddle brown hands */
#define FOREST_TEXT_COLOR 0x2F4F2F     /* Dark forest green text */

/**********************
 *      TYPEDEFS
 **********************/
typedef enum {
    ANIME_STYLE = 0,
    FASHION_STYLE,
    TECH_STYLE,
    FOREST_STYLE
} custom_watch_style_t;

typedef struct {
    lv_obj_t *screen;
    lv_obj_t *viewport;
    lv_obj_t *watch_face;
    lv_obj_t *hour_hand;
    lv_obj_t *minute_hand;
    lv_obj_t *second_hand;
    lv_obj_t *center_pin;
    lv_obj_t *decorative_elements[12]; /* For style-specific decorations */
    lv_timer_t *time_timer;
    custom_watch_style_t current_style;
} custom_watch_t;

/**********************
 *  STATIC VARIABLES
 **********************/
static custom_watch_t g_custom_watch;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void create_anime_style_face(void);
static void create_fashion_style_face(void);
static void create_tech_style_face(void);
static void create_forest_style_face(void);
static void update_watch_hands(void);
static void time_timer_cb(lv_timer_t *timer);

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * @brief Initialize custom watch with specified style
 * @param style The watch style to apply
 */
void custom_watch_init(custom_watch_style_t style)
{
    g_custom_watch.current_style = style;
    
    /* Create main screen */
    g_custom_watch.screen = lv_obj_create(NULL);
    lv_obj_set_size(g_custom_watch.screen, WATCH_SCREEN_WIDTH, WATCH_SCREEN_HEIGHT);
    lv_obj_center(g_custom_watch.screen);
    
    /* Create viewport */
    g_custom_watch.viewport = lv_obj_create(g_custom_watch.screen);
    lv_obj_set_size(g_custom_watch.viewport, WATCH_SCREEN_WIDTH, WATCH_SCREEN_HEIGHT);
    lv_obj_center(g_custom_watch.viewport);
    lv_obj_set_style_radius(g_custom_watch.viewport, WATCH_RADIUS, 0);
    lv_obj_set_style_clip_corner(g_custom_watch.viewport, true, 0);
    lv_obj_set_style_border_width(g_custom_watch.viewport, 0, 0);
    
    /* Apply style-specific background */
    switch (style) {
        case ANIME_STYLE:
            lv_obj_set_style_bg_color(g_custom_watch.viewport, lv_color_hex(ANIME_BG_COLOR), 0);
            create_anime_style_face();
            break;
        case FASHION_STYLE:
            lv_obj_set_style_bg_color(g_custom_watch.viewport, lv_color_hex(FASHION_BG_COLOR), 0);
            create_fashion_style_face();
            break;
        case TECH_STYLE:
            lv_obj_set_style_bg_color(g_custom_watch.viewport, lv_color_hex(TECH_BG_COLOR), 0);
            create_tech_style_face();
            break;
        case FOREST_STYLE:
            lv_obj_set_style_bg_color(g_custom_watch.viewport, lv_color_hex(FOREST_BG_COLOR), 0);
            create_forest_style_face();
            break;
    }
    
    lv_obj_set_style_bg_opa(g_custom_watch.viewport, LV_OPA_COVER, 0);
    
    /* Create time update timer */
    g_custom_watch.time_timer = lv_timer_create(time_timer_cb, 1000, NULL);
}

/**
 * @brief Create anime style watch face
 */
static void create_anime_style_face(void)
{
    /* Main watch face */
    g_custom_watch.watch_face = lv_obj_create(g_custom_watch.viewport);
    lv_obj_set_size(g_custom_watch.watch_face, WATCH_SCREEN_WIDTH - 10, WATCH_SCREEN_HEIGHT - 10);
    lv_obj_center(g_custom_watch.watch_face);
    lv_obj_set_style_radius(g_custom_watch.watch_face, (WATCH_SCREEN_WIDTH - 10) / 2, 0);
    lv_obj_set_style_bg_color(g_custom_watch.watch_face, lv_color_hex(ANIME_FACE_COLOR), 0);
    lv_obj_set_style_bg_opa(g_custom_watch.watch_face, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(g_custom_watch.watch_face, 3, 0);
    lv_obj_set_style_border_color(g_custom_watch.watch_face, lv_color_hex(ANIME_ACCENT_COLOR), 0);
    
    /* Add sparkle decorations */
    for (int i = 0; i < 8; i++) {
        float angle = i * 45.0f * DEG_TO_RAD;
        int x = (int)(cos(angle) * 80) + WATCH_SCREEN_WIDTH / 2;
        int y = (int)(sin(angle) * 80) + WATCH_SCREEN_HEIGHT / 2;
        
        lv_obj_t *sparkle = lv_obj_create(g_custom_watch.watch_face);
        lv_obj_set_size(sparkle, 4, 4);
        lv_obj_set_pos(sparkle, x - 2, y - 2);
        lv_obj_set_style_radius(sparkle, 2, 0);
        lv_obj_set_style_bg_color(sparkle, lv_color_hex(ANIME_ACCENT_COLOR), 0);
        lv_obj_set_style_bg_opa(sparkle, LV_OPA_80, 0);
    }
    
    /* Create hands */
    g_custom_watch.hour_hand = lv_obj_create(g_custom_watch.watch_face);
    lv_obj_set_size(g_custom_watch.hour_hand, 4, 40);
    lv_obj_set_style_bg_color(g_custom_watch.hour_hand, lv_color_hex(ANIME_HAND_COLOR), 0);
    lv_obj_set_style_radius(g_custom_watch.hour_hand, 2, 0);
    lv_obj_align(g_custom_watch.hour_hand, LV_ALIGN_CENTER, 0, -20);
    
    g_custom_watch.minute_hand = lv_obj_create(g_custom_watch.watch_face);
    lv_obj_set_size(g_custom_watch.minute_hand, 3, 55);
    lv_obj_set_style_bg_color(g_custom_watch.minute_hand, lv_color_hex(ANIME_HAND_COLOR), 0);
    lv_obj_set_style_radius(g_custom_watch.minute_hand, 1, 0);
    lv_obj_align(g_custom_watch.minute_hand, LV_ALIGN_CENTER, 0, -27);
    
    g_custom_watch.second_hand = lv_obj_create(g_custom_watch.watch_face);
    lv_obj_set_size(g_custom_watch.second_hand, 2, 60);
    lv_obj_set_style_bg_color(g_custom_watch.second_hand, lv_color_hex(ANIME_ACCENT_COLOR), 0);
    lv_obj_set_style_radius(g_custom_watch.second_hand, 1, 0);
    lv_obj_align(g_custom_watch.second_hand, LV_ALIGN_CENTER, 0, -30);
    
    /* Center pin */
    g_custom_watch.center_pin = lv_obj_create(g_custom_watch.watch_face);
    lv_obj_set_size(g_custom_watch.center_pin, 8, 8);
    lv_obj_center(g_custom_watch.center_pin);
    lv_obj_set_style_radius(g_custom_watch.center_pin, 4, 0);
    lv_obj_set_style_bg_color(g_custom_watch.center_pin, lv_color_hex(ANIME_ACCENT_COLOR), 0);
}

/**
 * @brief Create fashion style watch face
 */
static void create_fashion_style_face(void)
{
    /* Main watch face */
    g_custom_watch.watch_face = lv_obj_create(g_custom_watch.viewport);
    lv_obj_set_size(g_custom_watch.watch_face, WATCH_SCREEN_WIDTH - 10, WATCH_SCREEN_HEIGHT - 10);
    lv_obj_center(g_custom_watch.watch_face);
    lv_obj_set_style_radius(g_custom_watch.watch_face, (WATCH_SCREEN_WIDTH - 10) / 2, 0);
    lv_obj_set_style_bg_color(g_custom_watch.watch_face, lv_color_hex(FASHION_FACE_COLOR), 0);
    lv_obj_set_style_bg_opa(g_custom_watch.watch_face, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(g_custom_watch.watch_face, 2, 0);
    lv_obj_set_style_border_color(g_custom_watch.watch_face, lv_color_hex(FASHION_ACCENT_COLOR), 0);
    
    /* Add luxury hour markers */
    for (int i = 0; i < 12; i++) {
        float angle = i * 30.0f * DEG_TO_RAD;
        int x = (int)(cos(angle) * 90) + WATCH_SCREEN_WIDTH / 2;
        int y = (int)(sin(angle) * 90) + WATCH_SCREEN_HEIGHT / 2;
        
        lv_obj_t *marker = lv_obj_create(g_custom_watch.watch_face);
        lv_obj_set_size(marker, 6, 6);
        lv_obj_set_pos(marker, x - 3, y - 3);
        lv_obj_set_style_radius(marker, 3, 0);
        lv_obj_set_style_bg_color(marker, lv_color_hex(FASHION_ACCENT_COLOR), 0);
        lv_obj_set_style_bg_opa(marker, LV_OPA_90, 0);
    }
    
    /* Create elegant hands */
    g_custom_watch.hour_hand = lv_obj_create(g_custom_watch.watch_face);
    lv_obj_set_size(g_custom_watch.hour_hand, 5, 35);
    lv_obj_set_style_bg_color(g_custom_watch.hour_hand, lv_color_hex(FASHION_HAND_COLOR), 0);
    lv_obj_set_style_radius(g_custom_watch.hour_hand, 2, 0);
    lv_obj_align(g_custom_watch.hour_hand, LV_ALIGN_CENTER, 0, -17);
    
    g_custom_watch.minute_hand = lv_obj_create(g_custom_watch.watch_face);
    lv_obj_set_size(g_custom_watch.minute_hand, 4, 50);
    lv_obj_set_style_bg_color(g_custom_watch.minute_hand, lv_color_hex(FASHION_HAND_COLOR), 0);
    lv_obj_set_style_radius(g_custom_watch.minute_hand, 2, 0);
    lv_obj_align(g_custom_watch.minute_hand, LV_ALIGN_CENTER, 0, -25);
    
    g_custom_watch.second_hand = lv_obj_create(g_custom_watch.watch_face);
    lv_obj_set_size(g_custom_watch.second_hand, 2, 55);
    lv_obj_set_style_bg_color(g_custom_watch.second_hand, lv_color_hex(FASHION_ACCENT_COLOR), 0);
    lv_obj_set_style_radius(g_custom_watch.second_hand, 1, 0);
    lv_obj_align(g_custom_watch.second_hand, LV_ALIGN_CENTER, 0, -27);
    
    /* Center pin */
    g_custom_watch.center_pin = lv_obj_create(g_custom_watch.watch_face);
    lv_obj_set_size(g_custom_watch.center_pin, 10, 10);
    lv_obj_center(g_custom_watch.center_pin);
    lv_obj_set_style_radius(g_custom_watch.center_pin, 5, 0);
    lv_obj_set_style_bg_color(g_custom_watch.center_pin, lv_color_hex(FASHION_ACCENT_COLOR), 0);
}

/**
 * @brief Create tech style watch face
 */
static void create_tech_style_face(void)
{
    /* Main watch face */
    g_custom_watch.watch_face = lv_obj_create(g_custom_watch.viewport);
    lv_obj_set_size(g_custom_watch.watch_face, WATCH_SCREEN_WIDTH - 10, WATCH_SCREEN_HEIGHT - 10);
    lv_obj_center(g_custom_watch.watch_face);
    lv_obj_set_style_radius(g_custom_watch.watch_face, (WATCH_SCREEN_WIDTH - 10) / 2, 0);
    lv_obj_set_style_bg_color(g_custom_watch.watch_face, lv_color_hex(TECH_FACE_COLOR), 0);
    lv_obj_set_style_bg_opa(g_custom_watch.watch_face, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(g_custom_watch.watch_face, 2, 0);
    lv_obj_set_style_border_color(g_custom_watch.watch_face, lv_color_hex(TECH_ACCENT_COLOR), 0);
    
    /* Add digital-style hour markers */
    for (int i = 0; i < 12; i++) {
        float angle = i * 30.0f * DEG_TO_RAD;
        int x = (int)(cos(angle) * 85) + WATCH_SCREEN_WIDTH / 2;
        int y = (int)(sin(angle) * 85) + WATCH_SCREEN_HEIGHT / 2;
        
        lv_obj_t *marker = lv_obj_create(g_custom_watch.watch_face);
        lv_obj_set_size(marker, 8, 2);
        lv_obj_set_pos(marker, x - 4, y - 1);
        lv_obj_set_style_radius(marker, 1, 0);
        lv_obj_set_style_bg_color(marker, lv_color_hex(TECH_ACCENT_COLOR), 0);
        lv_obj_set_style_bg_opa(marker, LV_OPA_80, 0);
    }
    
    /* Create futuristic hands */
    g_custom_watch.hour_hand = lv_obj_create(g_custom_watch.watch_face);
    lv_obj_set_size(g_custom_watch.hour_hand, 4, 30);
    lv_obj_set_style_bg_color(g_custom_watch.hour_hand, lv_color_hex(TECH_HAND_COLOR), 0);
    lv_obj_set_style_radius(g_custom_watch.hour_hand, 2, 0);
    lv_obj_align(g_custom_watch.hour_hand, LV_ALIGN_CENTER, 0, -15);
    
    g_custom_watch.minute_hand = lv_obj_create(g_custom_watch.watch_face);
    lv_obj_set_size(g_custom_watch.minute_hand, 3, 45);
    lv_obj_set_style_bg_color(g_custom_watch.minute_hand, lv_color_hex(TECH_HAND_COLOR), 0);
    lv_obj_set_style_radius(g_custom_watch.minute_hand, 1, 0);
    lv_obj_align(g_custom_watch.minute_hand, LV_ALIGN_CENTER, 0, -22);
    
    g_custom_watch.second_hand = lv_obj_create(g_custom_watch.watch_face);
    lv_obj_set_size(g_custom_watch.second_hand, 2, 50);
    lv_obj_set_style_bg_color(g_custom_watch.second_hand, lv_color_hex(TECH_ACCENT_COLOR), 0);
    lv_obj_set_style_radius(g_custom_watch.second_hand, 1, 0);
    lv_obj_align(g_custom_watch.second_hand, LV_ALIGN_CENTER, 0, -25);
    
    /* Center pin */
    g_custom_watch.center_pin = lv_obj_create(g_custom_watch.watch_face);
    lv_obj_set_size(g_custom_watch.center_pin, 6, 6);
    lv_obj_center(g_custom_watch.center_pin);
    lv_obj_set_style_radius(g_custom_watch.center_pin, 3, 0);
    lv_obj_set_style_bg_color(g_custom_watch.center_pin, lv_color_hex(TECH_ACCENT_COLOR), 0);
}

/**
 * @brief Create forest style watch face
 */
static void create_forest_style_face(void)
{
    /* Main watch face */
    g_custom_watch.watch_face = lv_obj_create(g_custom_watch.viewport);
    lv_obj_set_size(g_custom_watch.watch_face, WATCH_SCREEN_WIDTH - 10, WATCH_SCREEN_HEIGHT - 10);
    lv_obj_center(g_custom_watch.watch_face);
    lv_obj_set_style_radius(g_custom_watch.watch_face, (WATCH_SCREEN_WIDTH - 10) / 2, 0);
    lv_obj_set_style_bg_color(g_custom_watch.watch_face, lv_color_hex(FOREST_FACE_COLOR), 0);
    lv_obj_set_style_bg_opa(g_custom_watch.watch_face, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(g_custom_watch.watch_face, 3, 0);
    lv_obj_set_style_border_color(g_custom_watch.watch_face, lv_color_hex(FOREST_ACCENT_COLOR), 0);
    
    /* Add leaf decorations */
    for (int i = 0; i < 12; i++) {
        float angle = i * 30.0f * DEG_TO_RAD;
        int x = (int)(cos(angle) * 85) + WATCH_SCREEN_WIDTH / 2;
        int y = (int)(sin(angle) * 85) + WATCH_SCREEN_HEIGHT / 2;
        
        lv_obj_t *leaf = lv_obj_create(g_custom_watch.watch_face);
        lv_obj_set_size(leaf, 6, 6);
        lv_obj_set_pos(leaf, x - 3, y - 3);
        lv_obj_set_style_radius(leaf, 3, 0);
        lv_obj_set_style_bg_color(leaf, lv_color_hex(FOREST_ACCENT_COLOR), 0);
        lv_obj_set_style_bg_opa(leaf, LV_OPA_70, 0);
    }
    
    /* Create natural hands */
    g_custom_watch.hour_hand = lv_obj_create(g_custom_watch.watch_face);
    lv_obj_set_size(g_custom_watch.hour_hand, 4, 35);
    lv_obj_set_style_bg_color(g_custom_watch.hour_hand, lv_color_hex(FOREST_HAND_COLOR), 0);
    lv_obj_set_style_radius(g_custom_watch.hour_hand, 2, 0);
    lv_obj_align(g_custom_watch.hour_hand, LV_ALIGN_CENTER, 0, -17);
    
    g_custom_watch.minute_hand = lv_obj_create(g_custom_watch.watch_face);
    lv_obj_set_size(g_custom_watch.minute_hand, 3, 50);
    lv_obj_set_style_bg_color(g_custom_watch.minute_hand, lv_color_hex(FOREST_HAND_COLOR), 0);
    lv_obj_set_style_radius(g_custom_watch.minute_hand, 1, 0);
    lv_obj_align(g_custom_watch.minute_hand, LV_ALIGN_CENTER, 0, -25);
    
    g_custom_watch.second_hand = lv_obj_create(g_custom_watch.watch_face);
    lv_obj_set_size(g_custom_watch.second_hand, 2, 55);
    lv_obj_set_style_bg_color(g_custom_watch.second_hand, lv_color_hex(FOREST_ACCENT_COLOR), 0);
    lv_obj_set_style_radius(g_custom_watch.second_hand, 1, 0);
    lv_obj_align(g_custom_watch.second_hand, LV_ALIGN_CENTER, 0, -27);
    
    /* Center pin */
    g_custom_watch.center_pin = lv_obj_create(g_custom_watch.watch_face);
    lv_obj_set_size(g_custom_watch.center_pin, 8, 8);
    lv_obj_center(g_custom_watch.center_pin);
    lv_obj_set_style_radius(g_custom_watch.center_pin, 4, 0);
    lv_obj_set_style_bg_color(g_custom_watch.center_pin, lv_color_hex(FOREST_ACCENT_COLOR), 0);
}

/**
 * @brief Update watch hands based on current time
 */
static void update_watch_hands(void)
{
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    
    int hour = tm_info->tm_hour % 12;
    int minute = tm_info->tm_min;
    int second = tm_info->tm_sec;
    
    float hour_angle = (hour * 30.0f) + (minute * 0.5f);
    float minute_angle = minute * 6.0f;
    float second_angle = second * 6.0f;
    
    /* Apply rotation to hands */
    lv_obj_set_style_transform_angle(g_custom_watch.hour_hand, (int16_t)(hour_angle * 10), 0);
    lv_obj_set_style_transform_pivot_x(g_custom_watch.hour_hand, 2, 0);
    lv_obj_set_style_transform_pivot_y(g_custom_watch.hour_hand, 20, 0);
    
    lv_obj_set_style_transform_angle(g_custom_watch.minute_hand, (int16_t)(minute_angle * 10), 0);
    lv_obj_set_style_transform_pivot_x(g_custom_watch.minute_hand, 1, 0);
    lv_obj_set_style_transform_pivot_y(g_custom_watch.minute_hand, 25, 0);
    
    lv_obj_set_style_transform_angle(g_custom_watch.second_hand, (int16_t)(second_angle * 10), 0);
    lv_obj_set_style_transform_pivot_x(g_custom_watch.second_hand, 1, 0);
    lv_obj_set_style_transform_pivot_y(g_custom_watch.second_hand, 27, 0);
}

/**
 * @brief Timer callback to update watch time
 */
static void time_timer_cb(lv_timer_t *timer)
{
    (void)timer;
    update_watch_hands();
}

/**
 * @brief Switch to a different watch style
 * @param style The new style to apply
 */
void custom_watch_switch_style(custom_watch_style_t style)
{
    if (g_custom_watch.current_style != style) {
        /* Clean up current style */
        if (g_custom_watch.watch_face) {
            lv_obj_del(g_custom_watch.watch_face);
        }
        
        /* Apply new style */
        g_custom_watch.current_style = style;
        
        switch (style) {
            case ANIME_STYLE:
                lv_obj_set_style_bg_color(g_custom_watch.viewport, lv_color_hex(ANIME_BG_COLOR), 0);
                create_anime_style_face();
                break;
            case FASHION_STYLE:
                lv_obj_set_style_bg_color(g_custom_watch.viewport, lv_color_hex(FASHION_BG_COLOR), 0);
                create_fashion_style_face();
                break;
            case TECH_STYLE:
                lv_obj_set_style_bg_color(g_custom_watch.viewport, lv_color_hex(TECH_BG_COLOR), 0);
                create_tech_style_face();
                break;
            case FOREST_STYLE:
                lv_obj_set_style_bg_color(g_custom_watch.viewport, lv_color_hex(FOREST_BG_COLOR), 0);
                create_forest_style_face();
                break;
        }
    }
}

/**
 * @brief Get the current watch style
 * @return Current watch style
 */
custom_watch_style_t custom_watch_get_style(void)
{
    return g_custom_watch.current_style;
}
