/**
 * @file rgb_led.h
 * @brief RGB LED control driver header
 *
 * This header file provides the interface for controlling WS2812B RGB LED
 * using GPIO P10 pin on T5AI development board.
 *
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 */

#ifndef RGB_LED_H
#define RGB_LED_H

#include "tuya_cloud_types.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief RGB LED color structure
 */
typedef struct {
    uint8_t red;     // Red component (0-255)
    uint8_t green;   // Green component (0-255)
    uint8_t blue;    // Blue component (0-255)
} rgb_color_t;

/**
 * @brief RGB LED configuration structure
 */
typedef struct {
    uint8_t brightness;  // Brightness level (0-100)
    rgb_color_t color;   // Current color
    uint8_t led_count;   // Number of LEDs in chain
} rgb_led_config_t;

/**
 * @brief Initialize RGB LED driver
 * 
 * @param gpio_pin GPIO pin number (P10 = GPIO_PIN_10)
 * @param led_count Number of LEDs in chain
 * @return int Success (0) or failure (-1)
 */
int rgb_led_init(uint8_t gpio_pin, uint8_t led_count);

/**
 * @brief Set RGB LED color
 * 
 * @param color RGB color structure
 * @return int Success (0) or failure (-1)
 */
int rgb_led_set_color(rgb_color_t color);

/**
 * @brief Set RGB LED brightness
 * 
 * @param brightness Brightness level (0-100)
 * @return int Success (0) or failure (-1)
 */
int rgb_led_set_brightness(uint8_t brightness);

/**
 * @brief Set RGB LED color and brightness
 * 
 * @param color RGB color structure
 * @param brightness Brightness level (0-100)
 * @return int Success (0) or failure (-1)
 */
int rgb_led_set_color_brightness(rgb_color_t color, uint8_t brightness);

/**
 * @brief Turn off all LEDs
 * 
 * @return int Success (0) or failure (-1)
 */
int rgb_led_off(void);

/**
 * @brief Get current RGB LED configuration
 * 
 * @param config Pointer to configuration structure
 * @return int Success (0) or failure (-1)
 */
int rgb_led_get_config(rgb_led_config_t *config);

/**
 * @brief Deinitialize RGB LED driver
 * 
 * @return int Success (0) or failure (-1)
 */
int rgb_led_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // RGB_LED_H
