/**
 * @file rgb_led.c
 * @brief RGB LED control driver implementation using PWM
 *
 * This source file provides the implementation for controlling WS2811F RGB LED
 * using PWM interface on T5AI development board.
 *
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 */

#include "rgb_led.h"
#include "tkl_pwm.h"
#include "tkl_pinmux.h"
#include "tuya_cloud_types.h"
#include <stdbool.h>
#include <string.h>
#include "tal_log.h"

// PWM configuration for WS2811F
#define WS2811F_PWM_FREQ    1000000  // 1MHz = 1μs per cycle
#define WS2811F_PWM_CYCLE   1000     // 1000 units per cycle (1μs)
#define WS2811F_PWM_PORT    TUYA_PWM_NUM_0  // Use PWM0

// WS2811F timing requirements (in nanoseconds)
// At 1μs per PWM cycle, we can control timing with duty cycle
#define WS2811F_T0H    350   // 0 bit high time (35% duty cycle)
#define WS2811F_T0L    800   // 0 bit low time (80% duty cycle)  
#define WS2811F_T1H    700   // 1 bit high time (70% duty cycle)
#define WS2811F_T1L    600   // 1 bit low time (60% duty cycle)

// Convert timing to PWM duty cycle
#define T0H_DUTY    ((WS2811F_T0H * WS2811F_PWM_CYCLE) / 1000)  // 350ns = 35% of 1μs
#define T0L_DUTY    ((WS2811F_T0L * WS2811F_PWM_CYCLE) / 1000)  // 800ns = 80% of 1μs
#define T1H_DUTY    ((WS2811F_T1H * WS2811F_PWM_CYCLE) / 1000)  // 700ns = 70% of 1μs
#define T1L_DUTY    ((WS2811F_T1L * WS2811F_PWM_CYCLE) / 1000)  // 600ns = 60% of 1μs

// Buffer size for PWM transmission
#define MAX_LED_COUNT        64  // Maximum number of LEDs supported
#define BITS_PER_LED         24  // 24 bits per LED (8 bits per color)

static uint8_t s_led_count = 0;
static rgb_led_config_t s_config = {0};
static bool s_pwm_initialized = false;

// Initialize PWM for WS2811F control
static int pwm_init_for_ws2811f(void)
{
    if (s_pwm_initialized) {
        return 0;
    }
    
    // Configure pin multiplexing for PWM0 (assuming P10 for output)
    tkl_io_pinmux_config(TUYA_IO_PIN_10, TUYA_PWM0);  // PWM0 output on P10
    
    // Configure PWM parameters for WS2811F
    TUYA_PWM_BASE_CFG_T cfg = {
        .polarity = TUYA_PWM_NEGATIVE,      // High active output
        .count_mode = TUYA_PWM_CNT_UP,     // Up counting mode
        .duty = T0H_DUTY,                  // Initial duty cycle (will be updated)
        .cycle = WS2811F_PWM_CYCLE,        // 1000 units per cycle
        .frequency = WS2811F_PWM_FREQ      // 1MHz
    };
    
    int ret = tkl_pwm_init(WS2811F_PWM_PORT, &cfg);
    if (ret != OPRT_OK) {
        PR_ERR("PWM init failed: %d", ret);
        return -1;
    }
    
    s_pwm_initialized = true;
    return 0;
}

// Send a single bit via PWM with proper timing
static int pwm_send_bit(uint8_t bit_value)
{
    TUYA_PWM_BASE_CFG_T cfg;
    
    if (bit_value == 1) {
        // 1码: 高电平700ns，低电平300ns
        cfg.duty = T1H_DUTY;  // 70% duty cycle
    } else {
        // 0码: 高电平350ns，低电平650ns
        cfg.duty = T0H_DUTY;  // 35% duty cycle
    }
    
    cfg.cycle = WS2811F_PWM_CYCLE;
    cfg.frequency = WS2811F_PWM_FREQ;
    cfg.polarity = TUYA_PWM_POSITIVE;
    cfg.count_mode = TUYA_PWM_CNT_UP;
    
    int ret = tkl_pwm_info_set(WS2811F_PWM_PORT, &cfg);
    if (ret != OPRT_OK) {
        return -1;
    }
    
    // Start PWM for one cycle
    ret = tkl_pwm_start(WS2811F_PWM_PORT);
    if (ret != OPRT_OK) {
        return -1;
    }
    
    // Wait for one cycle to complete (1μs)
    // Note: In real implementation, you might need to use timer or delay
    for (volatile int i = 0; i < 100; i++); // Simple delay
    
    // Stop PWM
    ret = tkl_pwm_stop(WS2811F_PWM_PORT);
    if (ret != OPRT_OK) {
        return -1;
    }
    
    return 0;
}

// Send colors to LEDs via PWM
static int send_colors_to_leds(void)
{
    if (s_led_count == 0) {
        return -1;
    }
    
    // Start PWM
    int ret = tkl_pwm_start(WS2811F_PWM_PORT);
    if (ret != OPRT_OK) {
        return -1;
    }
    
    // Send reset signal (50μs low)
    // This requires 50 PWM cycles with 0% duty cycle
    TUYA_PWM_BASE_CFG_T reset_cfg = {
        .polarity = TUYA_PWM_POSITIVE,
        .count_mode = TUYA_PWM_CNT_UP,
        .duty = 0,                    // 0% duty cycle = always low
        .cycle = WS2811F_PWM_CYCLE,
        .frequency = WS2811F_PWM_FREQ
    };
    
    ret = tkl_pwm_info_set(WS2811F_PWM_PORT, &reset_cfg);
    if (ret != OPRT_OK) {
        tkl_pwm_stop(WS2811F_PWM_PORT);
        return -1;
    }
    
    // Wait for reset signal (50μs)
    for (volatile int i = 0; i < 5000; i++);
    
    // Send LED data
    for (int led = 0; led < s_led_count; led++) {
        rgb_color_t color = s_config.color;
        
        // Apply brightness
        uint8_t red = (color.red * s_config.brightness) / 100;
        uint8_t green = (color.green * s_config.brightness) / 100;
        uint8_t blue = (color.blue * s_config.brightness) / 100;
        
        // WS2811F expects GRB order
        uint8_t colors[3] = {green, red, blue};
        
        // Send each color channel
        for (int color_idx = 0; color_idx < 3; color_idx++) {
            uint8_t color_value = colors[color_idx];
            
            // Send each bit of the color value
            for (int bit = 7; bit >= 0; bit--) {
                uint8_t bit_value = (color_value >> bit) & 0x01;
                
                if (pwm_send_bit(bit_value) != 0) {
                    tkl_pwm_stop(WS2811F_PWM_PORT);
                    return -1;
                }
            }
        }
    }
    
    // Stop PWM
    tkl_pwm_stop(WS2811F_PWM_PORT);
    
    return 0;
}

int rgb_led_init(uint8_t gpio_pin, uint8_t led_count)
{
    (void)gpio_pin;  // Not used in PWM mode
    
    if (led_count == 0 || led_count > MAX_LED_COUNT) {
        return -1;
    }
    
    // rgb_led_deinit();
    PR_ERR("----------------->rgb_led_init led_count: %d", led_count);
    s_led_count = led_count;
    
    // Initialize PWM for WS2811F
    int ret = pwm_init_for_ws2811f();
    if (ret != 0) {
        return -1;
    }
    
    // Initialize default configuration
    s_config.led_count = led_count;
    s_config.brightness = 100;  // Default 50% brightness
    s_config.color.red = 255;  // Default red color
    s_config.color.green = 0;
    s_config.color.blue = 0;
    
    // Turn off all LEDs initially
    return rgb_led_off();
}

int rgb_led_set_color(rgb_color_t color)
{
    if (s_led_count == 0) {
        return -1;
    }
    
    s_config.color = color;
    return send_colors_to_leds();
}

int rgb_led_set_brightness(uint8_t brightness)
{
    if (brightness > 100) {
        brightness = 100;
    }
    
    s_config.brightness = brightness;
    
    // Reapply current color with new brightness
    return send_colors_to_leds();
}

int rgb_led_set_color_brightness(rgb_color_t color, uint8_t brightness)
{
    s_config.color = color;
    s_config.brightness = brightness;
    
    return send_colors_to_leds();
}

int rgb_led_off(void)
{
    if (s_led_count == 0) {
        return -1;
    }
    
    // Set all colors to black
    rgb_color_t black = {0, 0, 0};
    s_config.color = black;
    
    return send_colors_to_leds();
}

int rgb_led_get_config(rgb_led_config_t *config)
{
    if (config == NULL) {
        return -1;
    }
    
    memcpy(config, &s_config, sizeof(rgb_led_config_t));
    return 0;
}

int rgb_led_deinit(void)
{
    // Turn off all LEDs
    rgb_led_off();
    
    // Deinitialize PWM if it was initialized
    if (s_pwm_initialized) {
        tkl_pwm_deinit(WS2811F_PWM_PORT);
        s_pwm_initialized = false;
    }
    
    s_led_count = 0;
    memset(&s_config, 0, sizeof(rgb_led_config_t));
    
    return 0;
}
