/**
 * @file ui_weather_clock.c
 * @brief Weather Clock UI Implementation
 *
 * This source file provides the implementation for the weather clock display,
 * including time display, date display, weather information, and real-time updates.
 *
 * Features:
 * - Time label scaled to 1.8x using LVGL transform (no new font assets required).
 * - Date/Weather line placed ABOVE the time label (size unchanged).
 * - Micro-adjust: time moved slightly UP; date/weather moved slightly DOWN.
 *
 * © 2021-2025 Tuya Inc.
 */

#include "tuya_cloud_types.h"
#include "ui_weather_clock.h"
#include "font_awesome_symbols.h"
#include "tal_log.h"
#include "tal_system.h"
#include "tal_time_service.h"
#include "tkl_memory.h"
#include "lvgl.h"
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

/***********************************************************
***********************macro / helpers**********************
***********************************************************/

/* 256 == 1.0x, 512 == 2.0x => 1.8x ≈ 256 * 1.8 = 461 */
#ifndef TIME_LABEL_ZOOM_1P8X
#define TIME_LABEL_ZOOM_1P8X 461
#endif


#ifndef UI_NUDGE_TIME_UP_PX
#define UI_NUDGE_TIME_UP_PX        6   
#endif
#ifndef UI_NUDGE_OTHERS_DOWN_PX
#define UI_NUDGE_OTHERS_DOWN_PX    6   
#endif

#ifndef lv_obj_get_content_width
#define lv_obj_get_content_width  lv_obj_get_width
#define lv_obj_get_content_height lv_obj_get_height
#endif

/***********************************************************
***********************variable define**********************
***********************************************************/
static WEATHER_CLOCK_T sg_weather_clock = {0};

/***********************************************************
***********************function define**********************
***********************************************************/


/**
 * @brief Simple time label update function
 * @param time_str New time string to display
 */
static void __update_time_display(const char *time_str)
{
    if (sg_weather_clock.ui.time_label == NULL || time_str == NULL) {
        return;
    }
    
    /* Simply update the text - LVGL handles centering automatically */
    lv_label_set_text(sg_weather_clock.ui.time_label, time_str);
    // PR_DEBUG("Updated time display: %s", time_str);
}

/**
 * @brief Get current time and format it as HH:MM:SS
 * @param time_str Buffer to store formatted time string
 * @param buffer_size Size of the buffer
 */
static void __get_current_time_string(char *time_str, int buffer_size)
{
    if (time_str == NULL || buffer_size < 9) {
        PR_ERR("Invalid parameters for time string");
        return;
    }

    time_t timestamp;

    /* Use independent time calculation if base timestamp is set */
    if (sg_weather_clock.base_timestamp > 0) {
        uint64_t now_ms  = (uint64_t)tal_time_get_posix_ms();
        uint64_t base_ms = (uint64_t)sg_weather_clock.base_uptime_ms;
        uint64_t elapsed_seconds = (now_ms - base_ms) / 1000ULL;
        timestamp = (time_t)(sg_weather_clock.base_timestamp + (time_t)elapsed_seconds);
        // PR_DEBUG("Using independent time calculation: base=%ld, elapsed=%llu, current=%ld",
        //          (long)sg_weather_clock.base_timestamp,
        //          (unsigned long long)elapsed_seconds,
        //          (long)timestamp);
    } else {
        /* No time sync received, show offline status */
        PR_DEBUG("No time sync received, showing offline status");
        strcpy(time_str, "OFFLINE");
        return;
    }

    /* Use Tuya's time service to get local time (handles timezone automatically) */
    POSIX_TM_S local_time;
    OPERATE_RET ret = tal_time_get_local_time_custom(timestamp, &local_time);
    if (ret == OPRT_OK) {
        snprintf(time_str, buffer_size, "%02d:%02d:%02d",
                 local_time.tm_hour, local_time.tm_min, local_time.tm_sec);
        // PR_DEBUG("Local time calculated: %02d:%02d:%02d (UTC timestamp: %ld)",
        //          local_time.tm_hour, local_time.tm_min, local_time.tm_sec, (long)timestamp);
    } else {
        /* Fallback to system localtime */
        struct tm *time_info = localtime(&timestamp);
        if (time_info != NULL) {
            snprintf(time_str, buffer_size, "%02d:%02d:%02d",
                     time_info->tm_hour, time_info->tm_min, time_info->tm_sec);
            PR_DEBUG("Using system localtime fallback: %02d:%02d:%02d",
                     time_info->tm_hour, time_info->tm_min, time_info->tm_sec);
        } else {
            PR_DEBUG("Failed to convert time to local time, using default");
            strcpy(time_str, "12:00:00");
        }
    }
}

/**
 * @brief Get current date and format it as MM/DD
 * @param date_str Buffer to store formatted date string
 * @param buffer_size Size of the buffer
 */
static void __get_current_date_string(char *date_str, int buffer_size)
{
    if (date_str == NULL || buffer_size < 6) {
        PR_ERR("Invalid parameters for date string");
        return;
    }

    time_t timestamp;

    /* Use independent time calculation if base timestamp is set */
    if (sg_weather_clock.base_timestamp > 0) {
        uint64_t now_ms  = (uint64_t)tal_time_get_posix_ms();
        uint64_t base_ms = (uint64_t)sg_weather_clock.base_uptime_ms;
        uint64_t elapsed_seconds = (now_ms - base_ms) / 1000ULL;
        timestamp = (time_t)(sg_weather_clock.base_timestamp + (time_t)elapsed_seconds);
    } else {
        /* No time sync received, date not available */
        PR_DEBUG("No time sync received, date not available");
        strcpy(date_str, "--/--");
        return;
    }

    /* Use Tuya's time service to get local time (handles timezone automatically) */
    POSIX_TM_S local_time;
    OPERATE_RET ret = tal_time_get_local_time_custom(timestamp, &local_time);
    if (ret == OPRT_OK) {
        snprintf(date_str, buffer_size, "%02d/%02d",
                 local_time.tm_mon + 1, local_time.tm_mday);
        // PR_DEBUG("Local date calculated: %02d/%02d (UTC timestamp: %ld)",
        //          local_time.tm_mon + 1, local_time.tm_mday, (long)timestamp);
    } else {
        /* Fallback to system localtime */
        struct tm *time_info = localtime(&timestamp);
        if (time_info != NULL) {
            snprintf(date_str, buffer_size, "%02d/%02d",
                     time_info->tm_mon + 1, time_info->tm_mday);
            PR_DEBUG("Using system localtime fallback for date: %02d/%02d",
                     time_info->tm_mon + 1, time_info->tm_mday);
        } else {
            PR_DEBUG("Failed to convert time to local time for date, using default");
            strcpy(date_str, "01/01");
        }
    }
}

/**
 * @brief Timer callback for updating time display
 * @param timer LVGL timer object
 */
static void __weather_clock_timer_cb(lv_timer_t *timer)
{
    LV_UNUSED(timer);

    if (!sg_weather_clock.is_visible) {
        PR_DEBUG("Weather clock not visible, skipping timer update");
        return;
    }

    if (sg_weather_clock.ui.time_label == NULL) {
        PR_ERR("Time label is NULL");
        return;
    }

    char time_str[16];
    char date_str[16];

    __get_current_time_string(time_str, sizeof(time_str));
    __get_current_date_string(date_str, sizeof(date_str));

    // PR_DEBUG("Updating time: %s, date: %s", time_str, date_str);

    /* Update time display */
    __update_time_display(time_str);

    /* Date/weather label only updates when real weather data is available */
    /* Timer only updates time, weather updates come from weather service */
}

/**
 * @brief Initialize minimal weather clock UI for 160x80 display
 * @param ui_font Font configuration
 * @return 0 on success, -1 on failure
 */
static int __ui_weather_clock_init_160x80(UI_FONT_T *ui_font)
{
    PR_DEBUG("Initializing minimal 160x80 weather clock layout...");

    if (ui_font == NULL) {
        PR_ERR("ui_font is NULL");
        return -1;
    }

    /* Initialize font configuration */
    sg_weather_clock.font = *ui_font;
    PR_DEBUG("Font configuration initialized");

    /* Validate font pointers */
    if (sg_weather_clock.font.text == NULL) {
        PR_ERR("Font text pointer is NULL");
        return -1;
    }
    if (sg_weather_clock.font.icon == NULL) {
        PR_ERR("Font icon pointer is NULL");
        return -1;
    }
    PR_DEBUG("Font pointers validated successfully");

    lv_obj_t *screen = lv_screen_active();
    if (screen == NULL) {
        PR_ERR("Failed to get active screen");
        return -1;
    }

    /* Set screen background to pure black for minimal design */
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    PR_DEBUG("Screen background configured to pure black");

    /* Main container with minimal styling - no borders, no shadows, no padding */
    sg_weather_clock.ui.container = lv_obj_create(screen);
    if (sg_weather_clock.ui.container == NULL) {
        PR_ERR("Failed to create main container");
        return -1;
    }
    lv_obj_set_size(sg_weather_clock.ui.container, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_pad_all(sg_weather_clock.ui.container, 0, 0);
    lv_obj_set_style_border_width(sg_weather_clock.ui.container, 0, 0);
    lv_obj_set_style_radius(sg_weather_clock.ui.container, 0, 0);
    lv_obj_set_style_bg_color(sg_weather_clock.ui.container, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(sg_weather_clock.ui.container, LV_OPA_COVER, 0);
    lv_obj_set_style_shadow_width(sg_weather_clock.ui.container, 0, 0);
    lv_obj_add_flag(sg_weather_clock.ui.container, LV_OBJ_FLAG_OVERFLOW_VISIBLE); 
    PR_DEBUG("Main container created with minimal styling");

    /* No status bar for minimal design - hide all status elements */
    sg_weather_clock.ui.status_bar = NULL;
    sg_weather_clock.ui.network_label = NULL;
    sg_weather_clock.ui.status_label = NULL;
    sg_weather_clock.ui.notification_label = NULL;

    /* Date & weather label - initially show date only */
    sg_weather_clock.ui.date_weather_label = lv_label_create(sg_weather_clock.ui.container);
    if (sg_weather_clock.ui.date_weather_label == NULL) {
        PR_ERR("Failed to create date weather label");
        return -1;
    }
    lv_obj_set_style_text_font(sg_weather_clock.ui.date_weather_label, sg_weather_clock.font.text, 0);
    lv_obj_set_style_text_color(sg_weather_clock.ui.date_weather_label, lv_color_hex(0x888888), 0);
    lv_obj_set_style_text_align(sg_weather_clock.ui.date_weather_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_letter_space(sg_weather_clock.ui.date_weather_label, 0, 0);
    lv_obj_set_width(sg_weather_clock.ui.date_weather_label, LV_HOR_RES);
    
    /* Initially show date only, weather will be added when available */
    char initial_date[16];
    __get_current_date_string(initial_date, sizeof(initial_date));
    lv_label_set_text(sg_weather_clock.ui.date_weather_label, initial_date);

    /* Main time display - ultra simple approach */
    sg_weather_clock.ui.time_label = lv_label_create(sg_weather_clock.ui.container);
    if (sg_weather_clock.ui.time_label == NULL) {
        PR_ERR("Failed to create time label");
        return -1;
    }
    
    /* Configure time label - show network prompt initially */
    lv_label_set_text(sg_weather_clock.ui.time_label, "OFFLINE");
    /* Use LVGL built-in font - much smaller memory footprint */
    lv_obj_set_style_text_font(sg_weather_clock.ui.time_label, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(sg_weather_clock.ui.time_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_align(sg_weather_clock.ui.time_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_letter_space(sg_weather_clock.ui.time_label, 1, 0);
    
    /* Simple center positioning - no scaling needed */
    lv_obj_set_width(sg_weather_clock.ui.time_label, LV_HOR_RES);  /* Full width for centering */
    lv_obj_align(sg_weather_clock.ui.time_label, LV_ALIGN_CENTER, 0, 0);

    /* Position date label above time label with perfect centering */
    lv_obj_align(sg_weather_clock.ui.date_weather_label,
                 LV_ALIGN_CENTER,
                 0,
                 -30);  /* Simple offset above time label */

    PR_DEBUG("Labels positioned: time centered (no scaling), date above time");

    /* Create update timer */
    sg_weather_clock.ui.update_timer = lv_timer_create(__weather_clock_timer_cb, WEATHER_CLOCK_UPDATE_INTERVAL_MS, NULL);
    if (sg_weather_clock.ui.update_timer == NULL) {
        PR_ERR("Failed to create weather clock update timer");
        return -1;
    }
    PR_DEBUG("Update timer created successfully");

    /* Initially show the weather clock (startup display) */
    sg_weather_clock.is_visible = TRUE;
    lv_obj_clear_flag(sg_weather_clock.ui.container, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(sg_weather_clock.ui.container);
    PR_DEBUG("Weather clock initially visible for startup");

    PR_DEBUG("Minimal weather clock UI initialized successfully for 160x80 display");
    return 0;
}

int ui_weather_clock_init(UI_FONT_T *ui_font)
{
    PR_DEBUG("Initializing weather clock UI...");

    if (ui_font == NULL) {
        PR_ERR("ui_font is NULL");
        return -1;
    }

    PR_DEBUG("Display resolution: %dx%d", LV_HOR_RES, LV_VER_RES);

    /* Use full initialization now that fonts are working */
    PR_DEBUG("Using full weather clock initialization");
    return __ui_weather_clock_init_160x80(ui_font);
}

void ui_weather_clock_show(void)
{
    PR_DEBUG("=== ATTEMPTING TO SHOW WEATHER CLOCK ===");

    if (sg_weather_clock.ui.container == NULL) {
        PR_ERR("Weather clock not initialized - container is NULL");
        PR_ERR("This means ui_weather_clock_init() either failed or was not called");
        return;
    }

    PR_DEBUG("Weather clock container exists, showing...");

    /* Hide other UI elements first by moving weather clock to front */
    lv_obj_move_foreground(sg_weather_clock.ui.container);

    /* Try to hide other siblings */
    lv_obj_t *screen = lv_screen_active();
    if (screen != NULL) {
        lv_obj_t *child = lv_obj_get_child(screen, 0);
        while (child != NULL) {
            if (child != sg_weather_clock.ui.container) {
                PR_DEBUG("Hiding other UI element");
                lv_obj_add_flag(child, LV_OBJ_FLAG_HIDDEN);
            }
            child = lv_obj_get_child(screen, lv_obj_get_index(child) + 1);
        }
    }

    lv_obj_clear_flag(sg_weather_clock.ui.container, LV_OBJ_FLAG_HIDDEN);
    sg_weather_clock.is_visible = TRUE;
    PR_DEBUG("Weather clock container made visible and moved to foreground");

    /* Start the update timer if it exists */
    if (sg_weather_clock.ui.update_timer != NULL) {
        lv_timer_resume(sg_weather_clock.ui.update_timer);
        PR_DEBUG("Update timer resumed");
    } else {
        PR_DEBUG("Update timer is NULL");
    }

    /* Update display immediately if time label exists */
    if (sg_weather_clock.ui.time_label != NULL) {
        char time_str[16];
        __get_current_time_string(time_str, sizeof(time_str));
        __update_time_display(time_str);
        PR_DEBUG("Initial time update completed: %s", time_str);

        /* Display will refresh automatically */
    } else {
        PR_DEBUG("Time label is NULL, skipping time update");
    }

    /* Date label is always shown, weather will be added when available */
    PR_DEBUG("Date label is always visible, weather will be added when available");

    PR_DEBUG("Weather clock shown successfully");
}

void ui_weather_clock_hide(void)
{
    PR_DEBUG("Attempting to hide weather clock...");

    if (sg_weather_clock.ui.container == NULL) {
        PR_ERR("Weather clock container is NULL");
        return;
    }

    lv_obj_add_flag(sg_weather_clock.ui.container, LV_OBJ_FLAG_HIDDEN);
    sg_weather_clock.is_visible = FALSE;
    PR_DEBUG("Weather clock container hidden");

    /* Pause the update timer */
    if (sg_weather_clock.ui.update_timer != NULL) {
        lv_timer_pause(sg_weather_clock.ui.update_timer);
        PR_DEBUG("Update timer paused");
    } else {
        PR_ERR("Update timer is NULL");
    }

    PR_DEBUG("Weather clock hidden successfully");
}

void ui_weather_clock_update_weather(const char *weather_icon, const char *temperature)
{
    PR_DEBUG("=== UI WEATHER UPDATE CALLED ===");
    PR_DEBUG("Input parameters:");
    PR_DEBUG("  - weather_icon: '%s'", weather_icon ? weather_icon : "NULL");
    PR_DEBUG("  - temperature: '%s'", temperature ? temperature : "NULL");

    if (!sg_weather_clock.is_visible) {
        PR_DEBUG("Weather clock not visible, skipping weather update");
        return;
    }

    if (sg_weather_clock.ui.date_weather_label == NULL) {
        PR_ERR("Date weather label is NULL");
        return;
    }

    /* Always show date, add weather if available */
    char date_str[16];
    __get_current_date_string(date_str, sizeof(date_str));
    
    if (weather_icon != NULL && temperature != NULL && 
        strlen(weather_icon) > 0 && strlen(temperature) > 0) {
        
        /* Format date and weather string */
        char date_weather_str[32];
        snprintf(date_weather_str, sizeof(date_weather_str), "%s  %s %s",
                 date_str, weather_icon, temperature);
        
        /* Update label with date and weather */
        lv_label_set_text(sg_weather_clock.ui.date_weather_label, date_weather_str);
        PR_DEBUG("Date and weather updated: '%s'", date_weather_str);
    } else {
        /* Show date only */
        lv_label_set_text(sg_weather_clock.ui.date_weather_label, date_str);
        PR_DEBUG("Date only updated: '%s'", date_str);
    }
    
    PR_DEBUG("=== UI WEATHER UPDATE COMPLETED ===");
}

void ui_weather_clock_update_network(const char *wifi_icon)
{
    /* Minimal design - no network display */
    PR_DEBUG("Network update ignored in minimal design: %s",
             wifi_icon ? wifi_icon : "NULL");
}

void ui_weather_clock_show_notification(const char *notification)
{
    /* Minimal design - no notification display */
    PR_DEBUG("Notification ignored in minimal design: %s",
             notification ? notification : "NULL");
}

BOOL_T ui_weather_clock_is_visible(void)
{
    return sg_weather_clock.is_visible;
}

void ui_weather_clock_set_timestamp(int timestamp)
{
    PR_DEBUG("Setting weather clock base timestamp: %d", timestamp);

    /* Set base timestamp and current uptime for independent time calculation */
    sg_weather_clock.base_timestamp = (time_t)timestamp;
    sg_weather_clock.base_uptime_ms = tal_time_get_posix_ms();

    PR_DEBUG("Base timestamp set to %ld, base uptime: %u ms",
             (long)sg_weather_clock.base_timestamp,
             (unsigned)sg_weather_clock.base_uptime_ms);

    /* Immediately update the display with the new time */
    ui_weather_clock_update_time();
}

void ui_weather_clock_update_time(void)
{
    if (!sg_weather_clock.is_visible || sg_weather_clock.ui.time_label == NULL) {
        return;
    }

    char time_str[16];
    char date_str[16];

    /* Get current time and date using independent calculation if set */
    __get_current_time_string(time_str, sizeof(time_str));
    __get_current_date_string(date_str, sizeof(date_str));

    /* Update time display */
    __update_time_display(time_str); 
    
    /* Date/weather label only updates when real weather data is available */
    /* Time update only affects time display, not weather */

    PR_DEBUG("Time updated: %s, date: %s", time_str, date_str);
}

void ui_weather_clock_cleanup(void)
{
    if (sg_weather_clock.ui.update_timer != NULL) {
        lv_timer_del(sg_weather_clock.ui.update_timer);
        sg_weather_clock.ui.update_timer = NULL;
    }

    if (sg_weather_clock.ui.container != NULL) {
        lv_obj_del(sg_weather_clock.ui.container);
        memset(&sg_weather_clock, 0, sizeof(WEATHER_CLOCK_T));
    }

    PR_DEBUG("Weather clock cleaned up");
}
