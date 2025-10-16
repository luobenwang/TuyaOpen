/**
 * @file watch_style_usage_example.h
 * @brief Header file for watch style usage example
 */

#ifndef __WATCH_STYLE_USAGE_EXAMPLE_H__
#define __WATCH_STYLE_USAGE_EXAMPLE_H__

#include "vintage_watch_app.h"

#ifdef __cplusplus
extern "C" {
#endif

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * @brief Initialize watch style system
 */
void watch_style_system_init(void);

/**
 * @brief Switch to next watch style
 */
void switch_to_next_style(void);

/**
 * @brief Switch to specific watch style
 * @param style Style number (0-3)
 */
void switch_to_style(int style);

/**
 * @brief Get current style name
 * @return Style name string
 */
const char* get_current_style_name(void);

/**
 * @brief Handle style change commands from user input
 * @param text User input text
 * @return 1 if style was changed, 0 otherwise
 */
int handle_style_commands(const char *text);

/**
 * @brief Get style-specific response for historical events
 * @param year The historical year
 * @param event The historical event
 * @return Style-appropriate response
 */
const char* get_style_response(const char *year, const char *event);

/**
 * @brief Print current style information
 */
void print_style_info(void);

#ifdef __cplusplus
}
#endif

#endif /* __WATCH_STYLE_USAGE_EXAMPLE_H__ */
