/**
 * @file watch_style_demo.c
 * @brief Demo application for different watch face styles
 * 
 * This file demonstrates how to use the 4 different watch face styles
 * and provides functions to switch between them.
 */

#include "watch_styles.h"
#include "ui_display.h"
#include "watch_style_demo.h"
#include <stdio.h>
#include <stdbool.h>

/**********************
 *  STATIC VARIABLES
 **********************/
static bool demo_initialized = false;

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * @brief Initialize the watch style demo
 */
void watch_style_demo_init(void)
{
    if (!demo_initialized) {
        /* Initialize with anime style by default */
        custom_watch_init(ANIME_STYLE);
        demo_initialized = true;
        printf("Watch style demo initialized with Anime style\n");
    }
}

/**
 * @brief Switch to anime style watch face
 */
void switch_to_anime_style(void)
{
    printf("Switching to Anime style watch face\n");
    custom_watch_switch_style(ANIME_STYLE);
}

/**
 * @brief Switch to fashion style watch face
 */
void switch_to_fashion_style(void)
{
    printf("Switching to Fashion style watch face\n");
    custom_watch_switch_style(FASHION_STYLE);
}

/**
 * @brief Switch to tech style watch face
 */
void switch_to_tech_style(void)
{
    printf("Switching to Tech style watch face\n");
    custom_watch_switch_style(TECH_STYLE);
}

/**
 * @brief Switch to forest style watch face
 */
void switch_to_forest_style(void)
{
    printf("Switching to Forest style watch face\n");
    custom_watch_switch_style(FOREST_STYLE);
}

/**
 * @brief Cycle through all watch styles
 */
void cycle_watch_styles(void)
{
    custom_watch_style_t current_style = custom_watch_get_style();
    custom_watch_style_t next_style;
    
    switch (current_style) {
        case ANIME_STYLE:
            next_style = FASHION_STYLE;
            break;
        case FASHION_STYLE:
            next_style = TECH_STYLE;
            break;
        case TECH_STYLE:
            next_style = FOREST_STYLE;
            break;
        case FOREST_STYLE:
            next_style = ANIME_STYLE;
            break;
        default:
            next_style = ANIME_STYLE;
            break;
    }
    
    custom_watch_switch_style(next_style);
}

/**
 * @brief Get current style name as string
 * @return String representation of current style
 */
const char* get_current_style_name(void)
{
    custom_watch_style_t current_style = custom_watch_get_style();
    
    switch (current_style) {
        case ANIME_STYLE:
            return "Anime Style";
        case FASHION_STYLE:
            return "Fashion Style";
        case TECH_STYLE:
            return "Tech Style";
        case FOREST_STYLE:
            return "Forest Style";
        default:
            return "Unknown Style";
    }
}

/**
 * @brief Print style information
 */
void print_style_info(void)
{
    printf("Current watch style: %s\n", get_current_style_name());
    printf("Available styles:\n");
    printf("1. Anime Style - Bright, colorful with anime-inspired design\n");
    printf("2. Fashion Style - Elegant, modern with luxury elements\n");
    printf("3. Tech Style - Futuristic, digital with neon accents\n");
    printf("4. Forest Style - Natural, organic with earth tones\n");
}
