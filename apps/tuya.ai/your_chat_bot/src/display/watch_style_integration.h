/**
 * @file watch_style_integration.h
 * @brief Header file for watch style integration with chat bot
 */

#ifndef __WATCH_STYLE_INTEGRATION_H__
#define __WATCH_STYLE_INTEGRATION_H__

#include "vintage_watch_app.h"

#ifdef __cplusplus
extern "C" {
#endif

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * @brief Initialize watch style integration
 */
void watch_style_integration_init(void);

/**
 * @brief Handle style change commands from user input
 * @param text User input text
 */
void handle_style_change_commands(const char *text);

/**
 * @brief Get style-specific response for historical events
 * @param year The historical year
 * @param event The historical event
 * @return Style-appropriate response
 */
const char* get_style_specific_response(const char *year, const char *event);

/**
 * @brief Enhanced user message handler with style integration
 * @param text User input text
 */
void enhanced_ui_set_user_msg(const char *text);

#ifdef __cplusplus
}
#endif

#endif /* __WATCH_STYLE_INTEGRATION_H__ */
