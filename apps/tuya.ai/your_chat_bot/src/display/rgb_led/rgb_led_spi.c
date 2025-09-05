/**
 * @file rgb_led_spi.c
 * @brief RGB LED control driver implementation using SPI
 *
 * This source file provides the implementation for controlling WS2811F RGB LED
 * using SPI interface on T5AI development board.
 *
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 */

#include "rgb_led.h"
#include "tkl_spi.h"
#include "tkl_pinmux.h"
#include "tuya_cloud_types.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "tal_log.h"

// SPI configuration for WS2811F
#define WS2811F_SPI_FREQ    4400000  // 8MHz SPI frequency
#define WS2811F_SPI_PORT    TUYA_SPI_NUM_1  // Use SPI1

// WS2811F timing requirements (in nanoseconds)
#define WS2811F_T0H    350   // 0 code high time
#define WS2811F_T0L    800   // 0 code low time
#define WS2811F_T1H    700   // 1 code high time
#define WS2811F_T1L    600   // 1 code low time

// At 8MHz SPI, each bit takes 1us (1 byte)
// We need to send one byte per bit to achieve the required timing
#define BYTES_PER_BIT    1   // 1 byte per bit at 8MHz SPI

#define MAX_LED_COUNT    64  // Maximum number of LEDs supported
#define BITS_PER_LED     24  // 24 bits per LED (8 bits per color)

// Global variables
static uint8_t s_led_count = 0;
static rgb_led_config_t s_config = {0};
static rgb_color_t s_led_colors[64] = {0};  // Support up to 64 LEDs
static bool s_spi_initialized = false;

// Initialize SPI for WS2811F control
static int spi_init_for_ws2811f(void)
{
    if (s_spi_initialized) {
        return 0;
    }
    
    // Configure pin multiplexing for SPI1
    tkl_io_pinmux_config(TUYA_IO_PIN_1, TUYA_SPI1_MISO);   // MISO
    tkl_io_pinmux_config(TUYA_IO_PIN_11, TUYA_SPI1_MOSI);  // MOSI (LED data)
    tkl_io_pinmux_config(TUYA_IO_PIN_12, TUYA_SPI1_CS);    // CS
    tkl_io_pinmux_config(TUYA_IO_PIN_13, TUYA_SPI1_CLK);   // CLK
    
    // Configure SPI parameters
    TUYA_SPI_BASE_CFG_T cfg = {
        .role = TUYA_SPI_ROLE_MASTER,      // Master mode
        .mode = TUYA_SPI_MODE0,            // SPI mode 0
        .type = TUYA_SPI_AUTO_TYPE,        // Hardware auto CS
        .databits = TUYA_SPI_DATA_BIT8,    // 8-bit data
        .bitorder = TUYA_SPI_ORDER_MSB2LSB, // MSB first
        .freq_hz = WS2811F_SPI_FREQ,        // 8MHz frequency
        .spi_dma_flags = 1,
    };
    
    int ret = tkl_spi_init(WS2811F_SPI_PORT, &cfg);
    if (ret != OPRT_OK) {
        PR_ERR("SPI init failed: %d", ret);
        return -1;
    }
    
    s_spi_initialized = true;
    return 0;
}

// Convert RGB color to WS2811F SPI data
// At 8MHz SPI, each byte = 1us
// 1码: 发送0xC0 (11000000b) - 高电平700ns，低电平300ns
// 0码: 发送0xFC (11111100b) - 高电平350ns，低电平650ns
static void rgb_to_ws2811f_spi_data(uint8_t red, uint8_t green, uint8_t blue, uint8_t *buffer)
{
    uint8_t colors[3] = {red, green, blue}; // WS2811F expects RGB order (not GRB)
    int byte_index = 0;
    
    for (int color_idx = 0; color_idx < 3; color_idx++) {
        uint8_t color_value = colors[color_idx];
        for (int bit = 7; bit >= 0; bit--) {
            uint8_t bit_value = (color_value >> bit) & 0x01;
            
            if (bit_value == 1) {
                // 1码: 发送0xC0 (11000000b)
                buffer[byte_index++] = 0xC0;
            } else {
                // 0码: 发送0xFC (11111100b)
                buffer[byte_index++] = 0xF0;
            }
        }
    }
}

// Send color data to LEDs via SPI
static int send_colors_to_leds(void)
{
    if (s_led_count == 0) {
        return -1;
    }
    
    // Calculate buffer size: each LED needs 24 bits * 8 bytes per bit
    int buffer_size = s_led_count * BITS_PER_LED * BYTES_PER_BIT;
    uint8_t *spi_buffer = malloc(buffer_size);
    if (!spi_buffer) {
        PR_ERR("Failed to allocate SPI buffer");
        return -1;
    }
    PR_ERR("------->send_colors,red: %d, green: %d, blue: %d,brightness: %d", s_led_colors[0].red, s_led_colors[0].green, s_led_colors[0].blue,s_config.brightness);
    // Convert RGB colors to SPI data
    int buffer_index = 0;
    for (int i = 0; i < s_led_count; i++) {
        uint8_t red = (s_led_colors[i].red * s_config.brightness) / 100;
        uint8_t green = (s_led_colors[i].green * s_config.brightness) / 100;
        uint8_t blue = (s_led_colors[i].blue * s_config.brightness) / 100;
        
        rgb_to_ws2811f_spi_data(red, green, blue, &spi_buffer[buffer_index]);
        
        buffer_index += BITS_PER_LED * BYTES_PER_BIT;
    }
    

    int ret = tkl_spi_send(WS2811F_SPI_PORT, spi_buffer, buffer_size);
    if (ret != OPRT_OK) {
        PR_ERR("SPI write failed: %d", ret);
        free(spi_buffer);
        return -1;
    }
    
    // Free buffer
    free(spi_buffer);
    
    // Reset pulse (low for >50us)
    // We'll send a series of 0x00 bytes to achieve this
    uint8_t reset_buffer[64];  // 64 bytes = 8us at 8MHz
    memset(reset_buffer, 0x00, sizeof(reset_buffer));
    tkl_spi_send(WS2811F_SPI_PORT, reset_buffer, sizeof(reset_buffer));
    
    return 0;
}

int rgb_led_init(uint8_t gpio_pin, uint8_t led_count)
{
    (void)gpio_pin;  // Not used in SPI mode
    
    if (led_count == 0 || led_count > MAX_LED_COUNT) {
        PR_ERR("Invalid LED count: %d", led_count);
        return -1;
    }
    
    s_led_count = led_count;
    s_config.brightness = 100;  // Default 100% brightness
    
    // Initialize SPI
    int ret = spi_init_for_ws2811f();
    if (ret != 0) {
        PR_ERR("SPI initialization failed");
        return -1;
    }
    
    // Initialize LED colors to off
    memset(s_led_colors, 0, sizeof(s_led_colors));
    
    PR_ERR("----------------->rgb_led_init led_count: %d", led_count);
    PR_NOTICE("RGB LED SPI driver initialized successfully");
    
    return 0;
}

int rgb_led_set_color(rgb_color_t color)
{
    if (s_led_count == 0) {
        return -1;
    }
    
    // Set color for first LED
    s_led_colors[0] = color;
    
    // Send colors to LEDs
    return send_colors_to_leds();
}

int rgb_led_set_brightness(uint8_t brightness)
{
    if (brightness > 100) {
        brightness = 100;
    }
    
    s_config.brightness = brightness;
    return send_colors_to_leds();
}

int rgb_led_set_color_brightness(rgb_color_t color, uint8_t brightness)
{
    if (s_led_count == 0) {
        return -1;
    }
    
    // Apply brightness to color
    rgb_color_t adjusted_color = {
        .red = (color.red * brightness) / 100,
        .green = (color.green * brightness) / 100,
        .blue = (color.blue * brightness) / 100
    };
    
    // Set color for first LED
    s_led_colors[0] = adjusted_color;
    
    // Send colors to LEDs
    return send_colors_to_leds();
}

int rgb_led_off(void)
{
    // Set all LEDs to off
    memset(s_led_colors, 0, sizeof(s_led_colors));
    return send_colors_to_leds();
}

int rgb_led_get_config(rgb_led_config_t *config)
{
    if (!config) {
        return -1;
    }
    
    *config = s_config;
    return 0;
}

int rgb_led_deinit(void)
{
    if (s_spi_initialized) {
        tkl_spi_deinit(WS2811F_SPI_PORT);
        s_spi_initialized = false;
    }
    
    s_led_count = 0;
    memset(&s_config, 0, sizeof(s_config));
    
    return 0;
}
