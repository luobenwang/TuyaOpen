/**
 * @file vintage_watch_app.h
 * Retro vintage pocket watch application for 466x466 circular OLED
 */

#ifndef VINTAGE_WATCH_APP_H
#define VINTAGE_WATCH_APP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../lvgl/lvgl.h"
#include <time.h>

/* Screen dimensions */
#ifndef WATCH_SCREEN_WIDTH
#define WATCH_SCREEN_WIDTH 240
#endif
#ifndef WATCH_SCREEN_HEIGHT
#define WATCH_SCREEN_HEIGHT 240
#endif

/* Watch face design constants */
#define WATCH_RADIUS (WATCH_SCREEN_WIDTH / 2)
#define WATCH_CENTER (WATCH_RADIUS)

/* Watch hands lengths (as percentage of radius) */
#define HOUR_HAND_LENGTH 0.6f
#define MINUTE_HAND_LENGTH 0.8f
#define SECOND_HAND_LENGTH 0.9f

/* Watch hands thickness */
#define HOUR_HAND_THICKNESS 8
#define MINUTE_HAND_THICKNESS 4
#define SECOND_HAND_THICKNESS 2

/* Color scheme - elegant vintage */
#define VINTAGE_BG_COLOR 0x1A0F08      /* Very dark brown background */
#define VINTAGE_FACE_COLOR 0xF8F8FF    /* Elegant off-white face */
#define VINTAGE_GOLD_COLOR 0xC9B037    /* Elegant gold */
#define VINTAGE_DARK_GOLD 0x8B7355     /* Sophisticated dark gold */
#define VINTAGE_TEXT_COLOR 0x2F2F2F    /* Elegant dark gray text */
#define VINTAGE_ACCENT_COLOR 0xE6E6FA  /* Lavender accent */
#define VINTAGE_SHADOW_COLOR 0x696969   /* Subtle shadow */
#define VINTAGE_HIGHLIGHT_COLOR 0xFFFFFF /* Pure white highlight */
#define VINTAGE_PLATINUM_COLOR 0xC0C0C0 /* Platinum accents */
#define VINTAGE_ROSE_GOLD 0xE8B4B8     /* Rose gold accents */

/* Watch modes */
typedef enum {
    WATCH_MODE_ANALOG = 0,
    WATCH_MODE_DIGITAL,
    WATCH_MODE_DUAL
} watch_mode_t;

/* Watch styles */
typedef enum {
    WATCH_STYLE_CLASSIC = 0,
    WATCH_STYLE_ART_DECO,
    WATCH_STYLE_VICTORIAN
} watch_style_t;

/* Main application functions */
void lv_demo_vintage_watch(void);
void vintage_watch_update_time(void);
void vintage_watch_set_mode(watch_mode_t mode);
void vintage_watch_set_style(watch_style_t style);

/* Time display functions */
void vintage_watch_show_date(void);
void vintage_watch_hide_date(void);
void vintage_watch_toggle_seconds(void);

/* Watch face customization */
void vintage_watch_set_face_color(uint32_t color);
void vintage_watch_set_hands_color(uint32_t color);
void vintage_watch_set_text_color(uint32_t color);

/* Animation controls */
void vintage_watch_start_smooth_hands(void);
void vintage_watch_stop_smooth_hands(void);
void vintage_watch_set_animation_speed(float speed);

/* Text display function */
void vintage_watch_show_text(const char *text);

/* Time sync function */
void vintage_watch_set_sync_time(time_t sync_time);

#ifdef __cplusplus
}
#endif

#endif /* VINTAGE_WATCH_APP_H */
