/**
 * @file ui_weather_clock.c
 * @brief Weather Clock UI Implementation
 *
 * This source file provides the implementation for the weather clock display,
 * including time display, date display, weather information, and real-time updates.
 *
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 *
 */

#include "tuya_cloud_types.h"
#include "ui_weather_clock.h"
#include "font_awesome_symbols.h"
#include "tal_log.h"
#include "tal_system.h"
#include "tkl_memory.h"
#include "lvgl.h"
#include <time.h>
#include <string.h>
#include <stdio.h>

/***********************************************************
***********************variable define**********************
***********************************************************/
static WEATHER_CLOCK_T sg_weather_clock = {0};

/***********************************************************
***********************function define**********************
***********************************************************/

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

    time_t timestamp = time(NULL);
    if (timestamp == (time_t)-1) {
        PR_DEBUG("System time not available, using default time");
        strcpy(time_str, "12:00:00");
        return;
    }
    
    struct tm *time_info = localtime(&timestamp);
    if (time_info != NULL) {
        snprintf(time_str, buffer_size, "%02d:%02d:%02d", 
                time_info->tm_hour, time_info->tm_min, time_info->tm_sec);
    } else {
        PR_DEBUG("Failed to convert time to local time, using default");
        strcpy(time_str, "12:00:00");
    }
}

/**
 * @brief Get current date and format it as MM/DD
 * @param date_str Buffer to store formatted date string
 * @param buffer_size Size of the buffer
 */
/**
 * @brief Get sun symbol - using ASCII "SUN" for compatibility
 * @return Sun symbol string
 */
static const char* __get_sun_symbol(void)
{
    return "SUN";
}

static void __get_current_date_string(char *date_str, int buffer_size)
{
    if (date_str == NULL || buffer_size < 6) {
        PR_ERR("Invalid parameters for date string");
        return;
    }

    time_t timestamp = time(NULL);
    if (timestamp == (time_t)-1) {
        PR_DEBUG("System time not available, using default date");
        strcpy(date_str, "01/01");
        return;
    }
    
    struct tm *time_info = localtime(&timestamp);
    if (time_info != NULL) {
        snprintf(date_str, buffer_size, "%02d/%02d", 
                time_info->tm_mon + 1, time_info->tm_mday);
    } else {
        PR_DEBUG("Failed to convert time to local time for date, using default");
        strcpy(date_str, "01/01");
    }
}

/**
 * @brief Timer callback for updating time display
 * @param timer LVGL timer object
 */
static void __weather_clock_timer_cb(lv_timer_t *timer)
{
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
    
    PR_DEBUG("Updating time: %s, date: %s", time_str, date_str);
    
    // Update time display
    lv_label_set_text(sg_weather_clock.ui.time_label, time_str);
    
    // Update date display (combine with weather info)
    if (sg_weather_clock.ui.date_weather_label != NULL) {
        char date_weather_str[32];
        const char* sun_symbol = __get_sun_symbol();
        snprintf(date_weather_str, sizeof(date_weather_str), "%s  %s 22°C", date_str, sun_symbol);
        lv_label_set_text(sg_weather_clock.ui.date_weather_label, date_weather_str);
        PR_DEBUG("Updated date/weather: %s", date_weather_str);
    } else {
        PR_ERR("Date weather label is NULL");
    }
    
    // Minimal design - no status bar updates needed
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

    // Initialize font configuration
    sg_weather_clock.font = *ui_font;
    PR_DEBUG("Font configuration initialized");
    
    // Validate font pointers
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
    
    // Set screen background to pure black for minimal design
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    PR_DEBUG("Screen background configured to pure black");

    // Main container with minimal styling - no borders, no shadows, no padding
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
    PR_DEBUG("Main container created with minimal styling");

    // No status bar for minimal design - hide all status elements
    sg_weather_clock.ui.status_bar = NULL;
    sg_weather_clock.ui.network_label = NULL;
    sg_weather_clock.ui.status_label = NULL;
    sg_weather_clock.ui.notification_label = NULL;

    // Main time display with pure white color and centered alignment
    sg_weather_clock.ui.time_label = lv_label_create(sg_weather_clock.ui.container);
    if (sg_weather_clock.ui.time_label == NULL) {
        PR_ERR("Failed to create time label");
        return -1;
    }
    lv_label_set_text(sg_weather_clock.ui.time_label, "00:00:00");
    lv_obj_set_style_text_font(sg_weather_clock.ui.time_label, sg_weather_clock.font.text, 0);
    lv_obj_set_style_text_color(sg_weather_clock.ui.time_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_align(sg_weather_clock.ui.time_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(sg_weather_clock.ui.time_label, LV_ALIGN_CENTER, 0, -10);
    lv_obj_set_style_text_letter_space(sg_weather_clock.ui.time_label, 1, 0);
    PR_DEBUG("Time label created with minimal white styling");

    // Date and weather display with medium gray color
    sg_weather_clock.ui.date_weather_label = lv_label_create(sg_weather_clock.ui.container);
    if (sg_weather_clock.ui.date_weather_label == NULL) {
        PR_ERR("Failed to create date weather label");
        return -1;
    }
    // Use the best available sun symbol
    const char* sun_symbol = __get_sun_symbol();
    char initial_text[32];
    snprintf(initial_text, sizeof(initial_text), "01/01  %s 22°C", sun_symbol);
    lv_label_set_text(sg_weather_clock.ui.date_weather_label, initial_text);
    lv_obj_set_style_text_font(sg_weather_clock.ui.date_weather_label, sg_weather_clock.font.text, 0);
    lv_obj_set_style_text_color(sg_weather_clock.ui.date_weather_label, lv_color_hex(0x888888), 0);
    lv_obj_set_style_text_align(sg_weather_clock.ui.date_weather_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(sg_weather_clock.ui.date_weather_label, LV_ALIGN_CENTER, 0, 10);
    lv_obj_set_style_text_letter_space(sg_weather_clock.ui.date_weather_label, 0, 0);
    PR_DEBUG("Date weather label created with minimal gray styling");

    // Create update timer
    sg_weather_clock.ui.update_timer = lv_timer_create(__weather_clock_timer_cb, WEATHER_CLOCK_UPDATE_INTERVAL_MS, NULL);
    if (sg_weather_clock.ui.update_timer == NULL) {
        PR_ERR("Failed to create weather clock update timer");
        return -1;
    }
    PR_DEBUG("Update timer created successfully");

    // Initially show the weather clock (startup display)
    sg_weather_clock.is_visible = TRUE;
    lv_obj_clear_flag(sg_weather_clock.ui.container, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(sg_weather_clock.ui.container);
    PR_DEBUG("Weather clock initially visible for startup");

    PR_DEBUG("Minimal weather clock UI initialized successfully for 160x80 display");
    return 0;
}

/*
 * @brief Minimal weather clock initialization for safety testing
 * @param ui_font Font configuration
 * @return 0 on success, -1 on failure
 * @note Currently unused - using full initialization instead
 */
/*
static int __ui_weather_clock_init_minimal(UI_FONT_T *ui_font)
{
    PR_DEBUG("=== STARTING MINIMAL WEATHER CLOCK INITIALIZATION ===");
    
    if (ui_font == NULL) {
        PR_ERR("ui_font is NULL");
        return -1;
    }

    // Validate font pointers
    if (ui_font->text == NULL) {
        PR_ERR("Font text pointer is NULL");
        return -1;
    }
    if (ui_font->icon == NULL) {
        PR_ERR("Font icon pointer is NULL");
        return -1;
    }
    PR_DEBUG("Font pointers validated successfully");

    // Get screen
    lv_obj_t *screen = lv_screen_active();
    if (screen == NULL) {
        PR_ERR("Failed to get active screen");
        return -1;
    }
    PR_DEBUG("Screen obtained successfully");

    // Initialize font configuration
    sg_weather_clock.font = *ui_font;
    PR_DEBUG("Font configuration initialized");

    // Create minimal container
    sg_weather_clock.ui.container = lv_obj_create(screen);
    if (sg_weather_clock.ui.container == NULL) {
        PR_ERR("Failed to create container");
        return -1;
    }
    PR_DEBUG("Container created successfully");

    // Set container size and style
    lv_obj_set_size(sg_weather_clock.ui.container, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_pad_all(sg_weather_clock.ui.container, 0, 0);
    lv_obj_set_style_border_width(sg_weather_clock.ui.container, 0, 0);
    lv_obj_set_style_bg_color(sg_weather_clock.ui.container, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(sg_weather_clock.ui.container, LV_OPA_COVER, 0);
    PR_DEBUG("Container configured successfully");

    // Create time label
    sg_weather_clock.ui.time_label = lv_label_create(sg_weather_clock.ui.container);
    if (sg_weather_clock.ui.time_label == NULL) {
        PR_ERR("Failed to create time label");
        return -1;
    }
    PR_DEBUG("Time label created successfully");

    // Set time label properties
    lv_label_set_text(sg_weather_clock.ui.time_label, "00:00:00");
    lv_obj_set_style_text_font(sg_weather_clock.ui.time_label, sg_weather_clock.font.text, 0);
    lv_obj_set_style_text_color(sg_weather_clock.ui.time_label, lv_color_black(), 0);
    lv_obj_set_style_text_align(sg_weather_clock.ui.time_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(sg_weather_clock.ui.time_label);
    PR_DEBUG("Time label configured successfully");

    // Initialize other UI components as NULL for now
    sg_weather_clock.ui.status_bar = NULL;
    sg_weather_clock.ui.network_label = NULL;
    sg_weather_clock.ui.notification_label = NULL;
    sg_weather_clock.ui.status_label = NULL;
    sg_weather_clock.ui.date_weather_label = NULL;
    sg_weather_clock.ui.update_timer = NULL;

    // For testing: show container immediately instead of hiding it
    // lv_obj_add_flag(sg_weather_clock.ui.container, LV_OBJ_FLAG_HIDDEN);
    sg_weather_clock.is_visible = TRUE;
    PR_DEBUG("Weather clock initially VISIBLE for testing");
    
    // Set test text immediately
    if (sg_weather_clock.ui.time_label != NULL) {
        lv_label_set_text(sg_weather_clock.ui.time_label, "INIT TEST");
        PR_DEBUG("Test text set during initialization: INIT TEST");
    }

    PR_DEBUG("=== MINIMAL WEATHER CLOCK INITIALIZATION COMPLETED SUCCESSFULLY ===");
    return 0;
}
*/

int ui_weather_clock_init(UI_FONT_T *ui_font)
{
    PR_DEBUG("Initializing weather clock UI...");
    
    if (ui_font == NULL) {
        PR_ERR("ui_font is NULL");
        return -1;
    }

    PR_DEBUG("Display resolution: %dx%d", LV_HOR_RES, LV_VER_RES);
    
    // Use full initialization now that fonts are working
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
    
    // Hide other UI elements first by moving weather clock to front
    lv_obj_move_foreground(sg_weather_clock.ui.container);
    
    // Try to hide the emoji UI by finding and hiding its container
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
    
    // Start the update timer if it exists
    if (sg_weather_clock.ui.update_timer != NULL) {
        lv_timer_resume(sg_weather_clock.ui.update_timer);
        PR_DEBUG("Update timer resumed");
    } else {
        PR_DEBUG("Update timer is NULL");
    }
    
    // Update display immediately if time label exists
    if (sg_weather_clock.ui.time_label != NULL) {
        char time_str[16];
        __get_current_time_string(time_str, sizeof(time_str));
        lv_label_set_text(sg_weather_clock.ui.time_label, time_str);
        PR_DEBUG("Initial time update completed: %s", time_str);
        
        // Force refresh the display
        lv_obj_invalidate(sg_weather_clock.ui.time_label);
        lv_obj_invalidate(sg_weather_clock.ui.container);
        PR_DEBUG("Display refresh triggered");
    } else {
        PR_DEBUG("Time label is NULL, skipping time update");
    }
    
    // Update date and weather if labels exist
    if (sg_weather_clock.ui.date_weather_label != NULL) {
        char date_str[32];
        __get_current_date_string(date_str, sizeof(date_str));
        char weather_text[64];
        const char* sun_symbol = __get_sun_symbol();
        snprintf(weather_text, sizeof(weather_text), "%s  %s 22°C", date_str, sun_symbol);
        lv_label_set_text(sg_weather_clock.ui.date_weather_label, weather_text);
        PR_DEBUG("Initial date/weather update completed: %s", weather_text);
    }
    
    // Minimal design - no status bar updates needed
    
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
    
    // Pause the update timer
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
    PR_DEBUG("Updating weather: icon=%s, temp=%s", 
             weather_icon ? weather_icon : "NULL", 
             temperature ? temperature : "NULL");
    
    if (!sg_weather_clock.is_visible) {
        PR_DEBUG("Weather clock not visible, skipping weather update");
        return;
    }
    
    if (sg_weather_clock.ui.date_weather_label == NULL) {
        PR_ERR("Date weather label is NULL");
        return;
    }

    char date_str[16];
    __get_current_date_string(date_str, sizeof(date_str));
    
    char date_weather_str[32];
    if (weather_icon != NULL && temperature != NULL) {
        snprintf(date_weather_str, sizeof(date_weather_str), "%s  %s %s", 
                date_str, weather_icon, temperature);
    } else {
        const char* sun_symbol = __get_sun_symbol();
        snprintf(date_weather_str, sizeof(date_weather_str), "%s  %s 22°C", date_str, sun_symbol);
    }
    
    lv_label_set_text(sg_weather_clock.ui.date_weather_label, date_weather_str);
    PR_DEBUG("Weather updated to: %s", date_weather_str);
}

void ui_weather_clock_update_network(const char *wifi_icon)
{
    // Minimal design - no network display
    PR_DEBUG("Network update ignored in minimal design: %s", 
             wifi_icon ? wifi_icon : "NULL");
}

void ui_weather_clock_show_notification(const char *notification)
{
    // Minimal design - no notification display
    PR_DEBUG("Notification ignored in minimal design: %s", 
             notification ? notification : "NULL");
}

BOOL_T ui_weather_clock_is_visible(void)
{
    return sg_weather_clock.is_visible;
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
