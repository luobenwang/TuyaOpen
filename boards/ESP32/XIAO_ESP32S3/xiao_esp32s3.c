/**
 * @file xiao_esp32s3.c
 * @brief Board-level hardware registration for XIAO ESP32S3.
 *
 * @copyright Copyright (c) 2021-2026 Tuya Inc. All Rights Reserved.
 */

#include "tuya_cloud_types.h"
#include "board_config.h"

#include "tal_api.h"
#include "tdd_audio_no_codec.h"
#include "tdd_button_gpio.h"
#include "tdd_led_gpio.h"

#include "board_com_api.h"

#if defined(BOARD_DISPLAY_TYPE) && (BOARD_DISPLAY_TYPE == DISPLAY_TYPE_OLED_SSD1306)
#include "oled_ssd1306.h"
#endif

/***********************************************************
***********************function define**********************
***********************************************************/

static OPERATE_RET __board_register_audio(void)
{
    OPERATE_RET rt = OPRT_OK;

#if defined(AUDIO_CODEC_NAME)
    TDD_AUDIO_NO_CODEC_T cfg = {0};
    cfg.i2s_id = 0;
    cfg.mic_sample_rate = 16000;
    cfg.spk_sample_rate = 16000;

    TUYA_CALL_ERR_RETURN(tdd_audio_no_codec_register(AUDIO_CODEC_NAME, cfg));
#endif

    return rt;
}

static OPERATE_RET __board_register_button(void)
{
    OPERATE_RET rt = OPRT_OK;

#if defined(BUTTON_NAME)
    BUTTON_GPIO_CFG_T button_hw_cfg = {
        .pin   = BOARD_BUTTON_PIN,
        .level = BOARD_BUTTON_ACTIVE_LV,
        .mode  = BUTTON_TIMER_SCAN_MODE,
        .pin_type.gpio_pull = TUYA_GPIO_PULLUP,
    };

    TUYA_CALL_ERR_RETURN(tdd_gpio_button_register(BUTTON_NAME, &button_hw_cfg));
#endif

    return rt;
}

static OPERATE_RET __board_register_led(void)
{
    OPERATE_RET rt = OPRT_OK;

#if defined(LED_NAME)
    TDD_LED_GPIO_CFG_T led_gpio = {0};

    led_gpio.pin = BOARD_LED_PIN;
    led_gpio.level = BOARD_LED_ACTIVE_LV;
    led_gpio.mode = TUYA_GPIO_PUSH_PULL;

    TUYA_CALL_ERR_RETURN(tdd_led_gpio_register(LED_NAME, &led_gpio));
#endif

    return rt;
}

/**
 * @brief Registers all the hardware peripherals (audio, button, LED) on the board.
 *
 * @return Returns OPERATE_RET_OK on success, or an appropriate error code on failure.
 */
OPERATE_RET board_register_hardware(void)
{
    OPERATE_RET rt = OPRT_OK;

    TUYA_CALL_ERR_LOG(__board_register_audio());
    TUYA_CALL_ERR_LOG(__board_register_button());
    TUYA_CALL_ERR_LOG(__board_register_led());

    return rt;
}

/**
 * @brief Initialize board display hardware.
 * @return 0 on success, negative value when display is not available.
 */
int board_display_init(void)
{
#if defined(BOARD_DISPLAY_TYPE) && (BOARD_DISPLAY_TYPE == DISPLAY_TYPE_OLED_SSD1306)
    return oled_ssd1306_init();
#else
    return -1;
#endif
}

/**
 * @brief Get display panel IO handle.
 * @return panel IO handle, NULL if display is not available.
 */
void *board_display_get_panel_io_handle(void)
{
#if defined(BOARD_DISPLAY_TYPE) && (BOARD_DISPLAY_TYPE == DISPLAY_TYPE_OLED_SSD1306)
    return oled_ssd1306_get_panel_io_handle();
#else
    return NULL;
#endif
}

/**
 * @brief Get display panel handle.
 * @return panel handle, NULL if display is not available.
 */
void *board_display_get_panel_handle(void)
{
#if defined(BOARD_DISPLAY_TYPE) && (BOARD_DISPLAY_TYPE == DISPLAY_TYPE_OLED_SSD1306)
    return oled_ssd1306_get_panel_handle();
#else
    return NULL;
#endif
}
