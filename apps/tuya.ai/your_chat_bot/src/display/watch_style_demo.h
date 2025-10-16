/**
 * @file watch_style_demo.h
 * @brief Header file for watch style demo application
 */

#ifndef __WATCH_STYLE_DEMO_H__
#define __WATCH_STYLE_DEMO_H__

#include "vintage_watch_app.h"

#ifdef __cplusplus
extern "C" {
#endif

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * @brief Initialize the watch style demo
 */
void watch_style_demo_init(void);

/**
 * @brief Switch to anime style watch face
 */
void switch_to_anime_style(void);

/**
 * @brief Switch to fashion style watch face
 */
void switch_to_fashion_style(void);

/**
 * @brief Switch to tech style watch face
 */
void switch_to_tech_style(void);

/**
 * @brief Switch to forest style watch face
 */
void switch_to_forest_style(void);

/**
 * @brief Cycle through all watch styles
 */
void cycle_watch_styles(void);

/**
 * @brief Get current style name as string
 * @return String representation of current style
 */
const char* get_current_style_name(void);

/**
 * @brief Print style information
 */
void print_style_info(void);

#ifdef __cplusplus
}
#endif

#endif /* __WATCH_STYLE_DEMO_H__ */
