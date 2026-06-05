/**
 * @file board_config.h
 * @brief Board-level peripheral configuration for XIAO ESP32S3.
 * @version 0.1
 * @date 2026-06-02
 */

#ifndef __BOARD_CONFIG_H__
#define __BOARD_CONFIG_H__

#include "sdkconfig.h"
#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
************************macro define************************
***********************************************************/

/* XIAO ESP32S3 serial pins */
#define UART0_TX_PIN (43)
#define UART0_RX_PIN (44)

/* XIAO ESP32S3 BOOT button: GPIO0, active low */
#define BOARD_BUTTON_PIN       TUYA_GPIO_NUM_0
#define BOARD_BUTTON_ACTIVE_LV TUYA_GPIO_LEVEL_LOW

/*
 * XIAO ESP32S3 user LED: GPIO21
 * The onboard LED is active-low (low level turns LED on).
 */
#define BOARD_LED_PIN       TUYA_GPIO_NUM_21
#define BOARD_LED_ACTIVE_LV TUYA_GPIO_LEVEL_LOW

/*
 * XIAO ESP32S3 Sense expansion board — onboard PDM microphone (scheme A).
 * CLK=GPIO42 (D11), DATA=GPIO41 (D12). Initialized in tkl_i2s when
 * CONFIG_BOARD_AUDIO_PDM_MIC is enabled.
 */
#define BOARD_PDM_MIC_CLK_GPIO 42
#define BOARD_PDM_MIC_DIN_GPIO 41

/*
 * MAX98357A I2S speaker on D3/D4/D5 (I2S1 TX):
 * BCLK=GPIO5 (D4), LRC=GPIO4 (D3), DIN=GPIO6 (D5). SD tied to 3V3.
 */
#define BOARD_I2S_SPK_BCLK_GPIO 5
#define BOARD_I2S_SPK_LRC_GPIO  4
#define BOARD_I2S_SPK_DIN_GPIO  6

/*
 * 0.96" SSD1306 OLED on I2C: SDA=GPIO1 (D0), SCL=GPIO2 (D1), addr 0x3C.
 */
#define OLED_I2C_PORT (0)
#define OLED_I2C_ADDR (0x3C)
#define OLED_I2C_SDA  (1)
#define OLED_I2C_SCL  (2)

#define OLED_WIDTH  (128)
#define OLED_HEIGHT (64)

#define DISPLAY_TYPE_UNKNOWN      0
#define DISPLAY_TYPE_OLED_SSD1306 1
#define DISPLAY_TYPE_LCD_SH8601   2

#define BOARD_DISPLAY_TYPE DISPLAY_TYPE_OLED_SSD1306

#define DISPLAY_WIDTH  OLED_WIDTH
#define DISPLAY_HEIGHT OLED_HEIGHT

#define DISPLAY_BUFFER_SIZE (DISPLAY_WIDTH * DISPLAY_HEIGHT)
#define DISPLAY_MONOCHROME  true

/* Landscape 128x64 (native SSD1306 orientation) */
#define DISPLAY_SWAP_XY   false
#define DISPLAY_MIRROR_X  true
#define DISPLAY_MIRROR_Y  true

/* RGB565 + monochrome=true (esp_lvgl_port SSD1306 path; UI must use blue-only for lit pixels) */
#define DISPLAY_COLOR_FORMAT LV_COLOR_FORMAT_RGB565

#define DISPLAY_BUFF_SPIRAM 0
#define DISPLAY_BUFF_DMA    1
#define DISPLAY_SWAP_BYTES  0

/***********************************************************
********************function declaration********************
***********************************************************/

/**
 * @brief Initialize board display hardware.
 * @return 0 on success, negative value when display is not available.
 */
int board_display_init(void);

/**
 * @brief Get display panel IO handle.
 * @return panel IO handle, NULL if display is not available.
 */
void *board_display_get_panel_io_handle(void);

/**
 * @brief Get display panel handle.
 * @return panel handle, NULL if display is not available.
 */
void *board_display_get_panel_handle(void);

#ifdef __cplusplus
}
#endif

#endif /* __BOARD_CONFIG_H__ */
