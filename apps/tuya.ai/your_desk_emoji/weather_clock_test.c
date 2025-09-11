/**
 * @file weather_clock_test.c
 * @brief Test program for weather clock functionality
 *
 * This is a simple test program to verify the weather clock implementation.
 * It can be used to test the weather clock display without the full system.
 *
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 *
 */

#include "tuya_cloud_types.h"
#include "app_display.h"
#include "ui_weather_clock.h"
#include "tal_log.h"
#include "tal_time.h"

/**
 * @brief Test weather clock functionality
 */
void test_weather_clock(void)
{
    PR_DEBUG("Starting weather clock test...");
    
    // Initialize display system
    OPERATE_RET ret = app_display_init();
    if (ret != OPRT_OK) {
        PR_ERR("Failed to initialize display system: %d", ret);
        return;
    }
    
    // Show weather clock
    app_display_send_msg(TY_DISPLAY_TP_WEATHER_CLOCK_SHOW, NULL, 0);
    PR_DEBUG("Weather clock shown");
    
    // Test weather update
    char weather_data[] = "☀️,25°C";
    app_display_send_msg(TY_DISPLAY_TP_WEATHER_CLOCK_UPDATE_WEATHER, 
                        (uint8_t *)weather_data, strlen(weather_data));
    PR_DEBUG("Weather updated to: %s", weather_data);
    
    // Wait for some time to see the clock
    tal_system_sleep(5000);
    
    // Test hiding weather clock
    app_display_send_msg(TY_DISPLAY_TP_WEATHER_CLOCK_HIDE, NULL, 0);
    PR_DEBUG("Weather clock hidden");
    
    // Test showing again
    tal_system_sleep(2000);
    app_display_send_msg(TY_DISPLAY_TP_WEATHER_CLOCK_SHOW, NULL, 0);
    PR_DEBUG("Weather clock shown again");
    
    PR_DEBUG("Weather clock test completed");
}

/**
 * @brief Test different weather conditions
 */
void test_weather_conditions(void)
{
    const char* weather_conditions[] = {
        "☀️,28°C",  // Sunny
        "⛅,22°C",  // Partly cloudy
        "🌧️,18°C",  // Rainy
        "❄️,5°C",   // Snowy
        "🌪️,15°C",  // Windy
    };
    
    PR_DEBUG("Testing different weather conditions...");
    
    for (int i = 0; i < 5; i++) {
        app_display_send_msg(TY_DISPLAY_TP_WEATHER_CLOCK_UPDATE_WEATHER, 
                            (uint8_t *)weather_conditions[i], strlen(weather_conditions[i]));
        PR_DEBUG("Weather set to: %s", weather_conditions[i]);
        tal_system_sleep(3000);
    }
    
    PR_DEBUG("Weather conditions test completed");
}
