/**
 * @file led_pixel_breath.c
 * @brief LED pixel breath and steady control
 * @version 0.1
 * @date 2026-05-08
 * @copyright Copyright (c) 2026 Tuya Inc. All Rights Reserved.
 */
#include "led_pixel_breath.h"

#if defined(ENABLE_LEDS_PIXEL) && (ENABLE_LEDS_PIXEL)
#include "tal_api.h"
#include "tal_sw_timer.h"
#include "tdd_pixel_ws2812.h"
#include "tdl_pixel_color_manage.h"
#include "tdl_pixel_dev_manage.h"

#include <string.h>

/* ---------------------------------------------------------------------------
 * Macros
 * --------------------------------------------------------------------------- */
#ifndef LEDS_PIXEL_NAME
#define LEDS_PIXEL_NAME "led_pixel"
#endif

/* Set to 1 for GRB strips, 0 for RGB strips. */
#ifndef LED_PIXEL_HW_ORDER_GRB
#define LED_PIXEL_HW_ORDER_GRB 1
#endif

/* Hardware defaults; can be overridden by compiler defines during porting. */
#ifndef LED_PIXELS_TOTAL_NUM
#define LED_PIXELS_TOTAL_NUM 24
#endif

#ifndef LED_PIXEL_COLOR_RES
#define LED_PIXEL_COLOR_RES 100
#endif

#ifndef LED_PIXEL_BREATH_STEP
#define LED_PIXEL_BREATH_STEP 1
#endif

#ifndef LED_PIXEL_BREATH_MS
#define LED_PIXEL_BREATH_MS 35
#endif

#ifndef LED_PIXEL_BREATH_INTENSITY_MIN
#define LED_PIXEL_BREATH_INTENSITY_MIN ((LED_PIXEL_COLOR_RES * 10) / 100)
#endif

/* ---------------------------------------------------------------------------
 * File-scope variables
 * --------------------------------------------------------------------------- */
static PIXEL_HANDLE_T s_led_pixels_handle = NULL;
static TIMER_ID       s_led_pixel_breath_tm = NULL;
static int32_t        s_led_pixel_intensity = 0;
static int32_t        s_led_pixel_direction = LED_PIXEL_BREATH_STEP;
static bool           s_led_pixel_hw_ready = false;
static PIXEL_COLOR_T  s_led_breath_peak = {0};

static const PIXEL_COLOR_T s_led_peak_white  = {.red = LED_PIXEL_COLOR_RES, .green = LED_PIXEL_COLOR_RES, .blue = LED_PIXEL_COLOR_RES};
static const PIXEL_COLOR_T s_led_peak_purple = {.red = LED_PIXEL_COLOR_RES, .green = 0, .blue = LED_PIXEL_COLOR_RES};
static const PIXEL_COLOR_T s_led_peak_red    = {.red = LED_PIXEL_COLOR_RES, .green = 0, .blue = 0};
static const PIXEL_COLOR_T s_led_peak_green  = {.red = 0, .green = LED_PIXEL_COLOR_RES, .blue = 0};
static const PIXEL_COLOR_T s_led_peak_cyan   = {.red = 0, .green = LED_PIXEL_COLOR_RES, .blue = (LED_PIXEL_COLOR_RES * 62) / 100};
static const PIXEL_COLOR_T s_led_peak_yellow = {.red = LED_PIXEL_COLOR_RES, .green = LED_PIXEL_COLOR_RES, .blue = 0};
static const PIXEL_COLOR_T s_led_peak_orange = {.red = LED_PIXEL_COLOR_RES, .green = (LED_PIXEL_COLOR_RES * 45) / 100, .blue = 0};
static const PIXEL_COLOR_T s_led_peak_pink   = {.red = LED_PIXEL_COLOR_RES, .green = (LED_PIXEL_COLOR_RES * 20) / 100, .blue = (LED_PIXEL_COLOR_RES * 35) / 100};

/* ---------------------------------------------------------------------------
 * Function declarations
 * --------------------------------------------------------------------------- */
static void __led_pixel_breath_timer_cb(TIMER_ID timer_id, void *arg);

/* ---------------------------------------------------------------------------
 * Function implementations
 * --------------------------------------------------------------------------- */
/**
 * @brief Register LED pixel hardware driver
 * @return OPRT_OK on success, error code otherwise
 * @note Called during board-level hardware registration.
 */
OPERATE_RET led_pixel_register_hardware(void)
{
    OPERATE_RET rt = OPRT_OK;

#if defined(ENABLE_SPI) && (ENABLE_SPI)
    char device_name[32] = "pixel";

    strncpy(device_name, LEDS_PIXEL_NAME, sizeof(device_name) - 1);
    device_name[sizeof(device_name) - 1] = '\0';

    PIXEL_DRIVER_CONFIG_T dev_init_cfg = {
        .port = TUYA_SPI_NUM_0,
        .line_seq = RGB_ORDER,
    };

    rt = tdd_ws2812_driver_register(device_name, &dev_init_cfg);
    if (rt == OPRT_OK) {
        PR_NOTICE("Pixel LED driver registered: %s", device_name);
    } else {
        PR_ERR("Failed to register pixel LED driver '%s': %d", device_name, rt);
    }
#endif

    return rt;
}

/**
 * @brief Convert one hex digit to nibble
 * @param[in] ch one hexadecimal character
 * @param[out] nibble parsed nibble value
 * @return true on success, false on invalid input
 */
static bool __hex_char_to_nibble(char ch, uint8_t *nibble)
{
    if (nibble == NULL) {
        return false;
    }

    if (ch >= '0' && ch <= '9') {
        *nibble = (uint8_t)(ch - '0');
        return true;
    }
    if (ch >= 'a' && ch <= 'f') {
        *nibble = (uint8_t)(ch - 'a' + 10);
        return true;
    }
    if (ch >= 'A' && ch <= 'F') {
        *nibble = (uint8_t)(ch - 'A' + 10);
        return true;
    }

    return false;
}

/**
 * @brief Parse a 6-digit RGB string to logical peak color
 * @param[in] rgb_hex six-digit RGB hex string, optionally prefixed with '#'
 * @param[out] peak parsed color on 0..LED_PIXEL_COLOR_RES scale
 * @return OPRT_OK on success, error code otherwise
 */
static OPERATE_RET __led_pixel_parse_rgb_hex(const char *rgb_hex, PIXEL_COLOR_T *peak)
{
    const char *hex = rgb_hex;
    uint8_t hi = 0;
    uint8_t lo = 0;
    uint8_t rgb[3] = {0};
    uint8_t i = 0;

    if (rgb_hex == NULL || peak == NULL) {
        return OPRT_INVALID_PARM;
    }

    if (rgb_hex[0] == '#') {
        hex = rgb_hex + 1;
    }
    if (strlen(hex) != 6) {
        return OPRT_INVALID_PARM;
    }

    for (i = 0; i < 3; i++) {
        if (!__hex_char_to_nibble(hex[i * 2], &hi) || !__hex_char_to_nibble(hex[i * 2 + 1], &lo)) {
            return OPRT_INVALID_PARM;
        }
        rgb[i] = (uint8_t)((hi << 4) | lo);
    }

    peak->red = (uint16_t)((uint32_t)rgb[0] * LED_PIXEL_COLOR_RES / 255U);
    peak->green = (uint16_t)((uint32_t)rgb[1] * LED_PIXEL_COLOR_RES / 255U);
    peak->blue = (uint16_t)((uint32_t)rgb[2] * LED_PIXEL_COLOR_RES / 255U);
    peak->cold = 0;
    peak->warm = 0;
    return OPRT_OK;
}

/**
 * @brief Convert logical RGB color to hardware color order
 * @param[in] logical_color logical RGB color
 * @param[out] hw_color hardware channel order color
 * @return OPRT_OK on success, error code otherwise
 */
static OPERATE_RET __led_pixel_convert_logical_to_hw_color(const PIXEL_COLOR_T *logical_color, PIXEL_COLOR_T *hw_color)
{
    if (logical_color == NULL || hw_color == NULL) {
        return OPRT_INVALID_PARM;
    }

#if defined(LED_PIXEL_HW_ORDER_GRB) && (LED_PIXEL_HW_ORDER_GRB == 1)
    hw_color->red = logical_color->green;
    hw_color->green = logical_color->red;
    hw_color->blue = logical_color->blue;
#else
    hw_color->red = logical_color->red;
    hw_color->green = logical_color->green;
    hw_color->blue = logical_color->blue;
#endif
    hw_color->cold = 0;
    hw_color->warm = 0;
    return OPRT_OK;
}

/**
 * @brief Lazy initialize LED pixel hardware and timer resources
 * @return OPRT_OK on success, error code otherwise
 */
static OPERATE_RET __led_pixel_breath_hw_init(void)
{
    OPERATE_RET rt = OPRT_OK;
    PIXEL_DEV_CONFIG_T pixels_cfg = {
        .pixel_num = LED_PIXELS_TOTAL_NUM,
        .pixel_resolution = LED_PIXEL_COLOR_RES,
    };

    if (s_led_pixel_hw_ready) {
        return OPRT_OK;
    }

    rt = tdl_pixel_dev_find(LEDS_PIXEL_NAME, &s_led_pixels_handle);
    if (rt != OPRT_OK || s_led_pixels_handle == NULL) {
        return (rt != OPRT_OK) ? rt : OPRT_COM_ERROR;
    }

    TUYA_CALL_ERR_RETURN(tdl_pixel_dev_open(s_led_pixels_handle, &pixels_cfg));
    TUYA_CALL_ERR_RETURN(tal_sw_timer_create(__led_pixel_breath_timer_cb, NULL, &s_led_pixel_breath_tm));

    s_led_pixel_hw_ready = true;
    return OPRT_OK;
}

/**
 * @brief Timer callback to render one breath animation frame
 * @param[in] timer_id software timer handle
 * @param[in] arg unused
 * @return none
 */
static void __led_pixel_breath_timer_cb(TIMER_ID timer_id, void *arg)
{
    PIXEL_COLOR_T logical_color = {0};
    PIXEL_COLOR_T hw_color = {0};

    (void)timer_id;
    (void)arg;

    if (s_led_pixels_handle == NULL) {
        return;
    }

    logical_color.red = (uint16_t)((int32_t)s_led_breath_peak.red * s_led_pixel_intensity / LED_PIXEL_COLOR_RES);
    logical_color.green = (uint16_t)((int32_t)s_led_breath_peak.green * s_led_pixel_intensity / LED_PIXEL_COLOR_RES);
    logical_color.blue = (uint16_t)((int32_t)s_led_breath_peak.blue * s_led_pixel_intensity / LED_PIXEL_COLOR_RES);

    if (__led_pixel_convert_logical_to_hw_color(&logical_color, &hw_color) == OPRT_OK) {
        OPERATE_RET rc = OPRT_OK;
        rc = tdl_pixel_set_single_color_all(s_led_pixels_handle, &hw_color);
        if (rc != OPRT_OK) {
            PR_ERR("tdl_pixel_set_single_color_all failed: %d", rc);
        }
        rc = tdl_pixel_dev_refresh(s_led_pixels_handle);
        if (rc != OPRT_OK) {
            PR_ERR("tdl_pixel_dev_refresh failed: %d", rc);
        }
    }

    s_led_pixel_intensity += s_led_pixel_direction;
    if (s_led_pixel_intensity >= LED_PIXEL_COLOR_RES) {
        s_led_pixel_intensity = LED_PIXEL_COLOR_RES;
        s_led_pixel_direction = -LED_PIXEL_BREATH_STEP;
    } else if (s_led_pixel_intensity <= LED_PIXEL_BREATH_INTENSITY_MIN) {
        s_led_pixel_intensity = LED_PIXEL_BREATH_INTENSITY_MIN;
        s_led_pixel_direction = LED_PIXEL_BREATH_STEP;
    }
}

/**
 * @brief Start breath animation with a given logical peak color
 * @param[in] peak logical peak color
 * @return OPRT_OK on success, error code otherwise
 */
static OPERATE_RET __led_pixel_breath_apply_peak(const PIXEL_COLOR_T *peak)
{
    OPERATE_RET rt = OPRT_OK;

    if (peak == NULL) {
        return OPRT_INVALID_PARM;
    }

    TUYA_CALL_ERR_RETURN(__led_pixel_breath_hw_init());

    s_led_breath_peak = *peak;
    s_led_breath_peak.cold = 0;
    s_led_breath_peak.warm = 0;
    s_led_pixel_intensity = LED_PIXEL_BREATH_INTENSITY_MIN;
    s_led_pixel_direction = LED_PIXEL_BREATH_STEP;

    if (s_led_pixel_breath_tm != NULL && tal_sw_timer_is_running(s_led_pixel_breath_tm) != FALSE) {
        OPERATE_RET rc = tal_sw_timer_stop(s_led_pixel_breath_tm);
        if (rc != OPRT_OK) {
            PR_ERR("tal_sw_timer_stop failed: %d", rc);
        }
    }

    TUYA_CALL_ERR_RETURN(tal_sw_timer_start(s_led_pixel_breath_tm, LED_PIXEL_BREATH_MS, TAL_TIMER_CYCLE));

    {
        OPERATE_RET rc = tal_sw_timer_trigger(s_led_pixel_breath_tm);
        if (rc != OPRT_OK) {
            PR_ERR("tal_sw_timer_trigger failed: %d", rc);
        }
    }

    return OPRT_OK;
}

/**
 * @brief Apply one steady logical color to all pixels
 * @param[in] peak logical color
 * @return OPRT_OK on success, error code otherwise
 */
static OPERATE_RET __led_pixel_set_steady_peak(const PIXEL_COLOR_T *peak)
{
    OPERATE_RET rt = OPRT_OK;
    PIXEL_COLOR_T hw_color = {0};

    if (peak == NULL) {
        return OPRT_INVALID_PARM;
    }

    TUYA_CALL_ERR_RETURN(__led_pixel_breath_hw_init());

    if (s_led_pixel_breath_tm != NULL && tal_sw_timer_is_running(s_led_pixel_breath_tm) != FALSE) {
        TUYA_CALL_ERR_RETURN(tal_sw_timer_stop(s_led_pixel_breath_tm));
    }
    TUYA_CALL_ERR_RETURN(__led_pixel_convert_logical_to_hw_color(peak, &hw_color));
    TUYA_CALL_ERR_RETURN(tdl_pixel_set_single_color_all(s_led_pixels_handle, &hw_color));
    TUYA_CALL_ERR_RETURN(tdl_pixel_dev_refresh(s_led_pixels_handle));

    s_led_breath_peak = *peak;
    s_led_pixel_intensity = LED_PIXEL_COLOR_RES;
    s_led_pixel_direction = LED_PIXEL_BREATH_STEP;
    return OPRT_OK;
}

/**
 * @brief Start default white breathing effect
 * @return OPRT_OK on success, error code otherwise
 */
OPERATE_RET led_pixel_breath_start_white(void)
{
    return __led_pixel_breath_apply_peak(&s_led_peak_white);
}

/**
 * @brief Start breathing using custom hex RGB color
 * @param[in] rgb_hex six-digit RGB hex string, optionally prefixed with '#'
 * @return OPRT_OK on success, error code otherwise
 */
OPERATE_RET led_pixel_breath_start_blue(const char *rgb_hex)
{
    OPERATE_RET rt = OPRT_OK;
    PIXEL_COLOR_T peak = {0};

    TUYA_CALL_ERR_RETURN(__led_pixel_parse_rgb_hex(rgb_hex, &peak));
    return __led_pixel_breath_apply_peak(&peak);
}

/**
 * @brief Apply RGB color with brightness in breath or steady mode
 * @param[in] rgb_hex six-digit RGB hex string, optionally prefixed with '#'
 * @param[in] brightness_pct brightness percentage in range 0..100
 * @param[in] is_breath true for breath mode, false for steady mode
 * @return OPRT_OK on success, error code otherwise
 */
OPERATE_RET led_pixel_apply_rgb_mode(const char *rgb_hex, uint8_t brightness_pct, bool is_breath)
{
    OPERATE_RET rt = OPRT_OK;
    PIXEL_COLOR_T peak = {0};
    PIXEL_COLOR_T scaled_peak = {0};
    uint32_t clamped_pct = brightness_pct;

    TUYA_CALL_ERR_RETURN(__led_pixel_parse_rgb_hex(rgb_hex, &peak));

    if (clamped_pct > 100U) {
        clamped_pct = 100U;
    }

    scaled_peak.red = (uint16_t)((uint32_t)peak.red * clamped_pct / 100U);
    scaled_peak.green = (uint16_t)((uint32_t)peak.green * clamped_pct / 100U);
    scaled_peak.blue = (uint16_t)((uint32_t)peak.blue * clamped_pct / 100U);

    if (clamped_pct == 0U) {
        return led_pixel_breath_stop();
    }
    if (is_breath) {
        return __led_pixel_breath_apply_peak(&scaled_peak);
    }

    return __led_pixel_set_steady_peak(&scaled_peak);
}

/**
 * @brief Start purple breathing
 * @return OPRT_OK on success, error code otherwise
 */
OPERATE_RET led_pixel_breath_start_purple(void)
{
    return __led_pixel_breath_apply_peak(&s_led_peak_purple);
}

/**
 * @brief Start red breathing
 * @return OPRT_OK on success, error code otherwise
 */
OPERATE_RET led_pixel_breath_start_red(void)
{
    return __led_pixel_breath_apply_peak(&s_led_peak_red);
}

/**
 * @brief Start green breathing
 * @return OPRT_OK on success, error code otherwise
 */
OPERATE_RET led_pixel_breath_start_green(void)
{
    return __led_pixel_breath_apply_peak(&s_led_peak_green);
}

/**
 * @brief Start cyan breathing
 * @return OPRT_OK on success, error code otherwise
 */
OPERATE_RET led_pixel_breath_start_cyan(void)
{
    return __led_pixel_breath_apply_peak(&s_led_peak_cyan);
}

/**
 * @brief Start yellow breathing
 * @return OPRT_OK on success, error code otherwise
 */
OPERATE_RET led_pixel_breath_start_yellow(void)
{
    return __led_pixel_breath_apply_peak(&s_led_peak_yellow);
}

/**
 * @brief Start orange breathing
 * @return OPRT_OK on success, error code otherwise
 */
OPERATE_RET led_pixel_breath_start_orange(void)
{
    return __led_pixel_breath_apply_peak(&s_led_peak_orange);
}

/**
 * @brief Start pink breathing
 * @return OPRT_OK on success, error code otherwise
 */
OPERATE_RET led_pixel_breath_start_pink(void)
{
    return __led_pixel_breath_apply_peak(&s_led_peak_pink);
}

/**
 * @brief Stop breathing and turn all pixels off
 * @return OPRT_OK on success, error code otherwise
 */
OPERATE_RET led_pixel_breath_stop(void)
{
    OPERATE_RET rc = OPRT_OK;
    PIXEL_COLOR_T off = {0};

    if (s_led_pixel_breath_tm != NULL && tal_sw_timer_is_running(s_led_pixel_breath_tm) != FALSE) {
        rc = tal_sw_timer_stop(s_led_pixel_breath_tm);
        if (rc != OPRT_OK) {
            PR_ERR("tal_sw_timer_stop failed: %d", rc);
        }
    }

    if (s_led_pixels_handle != NULL) {
        rc = tdl_pixel_set_single_color_all(s_led_pixels_handle, &off);
        if (rc != OPRT_OK) {
            PR_ERR("tdl_pixel_set_single_color_all failed: %d", rc);
        }
        rc = tdl_pixel_dev_refresh(s_led_pixels_handle);
        if (rc != OPRT_OK) {
            PR_ERR("tdl_pixel_dev_refresh failed: %d", rc);
        }
    }

    s_led_pixel_intensity = 0;
    return OPRT_OK;
}
#endif /* ENABLE_LEDS_PIXEL */
