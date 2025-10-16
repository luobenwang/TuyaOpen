/**
 * @file watch_styles.h
 * @brief Header file for custom watch face styles
 * 
 * This header provides declarations for 4 different watch face styles:
 * 1. Anime Style - Bright, colorful with anime-inspired design
 * 2. Fashion Style - Elegant, modern with luxury elements
 * 3. Tech Style - Futuristic, digital with neon accents
 * 4. Forest Style - Natural, organic with earth tones
 */

#ifndef __WATCH_STYLES_H__
#define __WATCH_STYLES_H__

#include "vintage_watch_app.h"

#ifdef __cplusplus
extern "C" {
#endif

/**********************
 *      TYPEDEFS
 **********************/
typedef enum {
    ANIME_STYLE = 0,
    FASHION_STYLE,
    TECH_STYLE,
    FOREST_STYLE
} custom_watch_style_t;

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * @brief Initialize custom watch with specified style
 * @param style The watch style to apply
 */
void custom_watch_init(custom_watch_style_t style);

/**
 * @brief Switch to a different watch style
 * @param style The new style to apply
 */
void custom_watch_switch_style(custom_watch_style_t style);

/**
 * @brief Get the current watch style
 * @return Current watch style
 */
custom_watch_style_t custom_watch_get_style(void);

#ifdef __cplusplus
}
#endif

#endif /* __WATCH_STYLES_H__ */
