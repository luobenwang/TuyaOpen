/**
 * @file app_tof.c
 * @brief VL53L0X TOF distance on I2C0 (exclusive init by default; optional share with gesture)
 * @version 1.1
 * @date 2026-05-11
 * @copyright Copyright (c) 2026 Tuya Inc. All Rights Reserved.
 */

#include "app_tof.h"

#include "app_i2c0_lock.h"
#include "app_water_stats.h"

#include "ai_agent.h"

#include "tal_api.h"
#include "tal_thread.h"
#include "tal_system.h"

#include "tkl_i2c.h"
#include "tkl_pinmux.h"

#include "lv_vendor.h"

/* ---------------------------------------------------------------------------
 * Macros
 * --------------------------------------------------------------------------- */
/** I2C port for VL53L0X */
#define APP_TOF_I2C_PORT TUYA_I2C_NUM_0

/**
 * Set to 1 only if app_gesture_init() already called tkl_i2c_init on this port
 * (same pins). Default 0: TOF performs pinmux + tkl_i2c_init itself.
 */
#ifndef APP_TOF_SHARE_I2C0_BUS
#define APP_TOF_SHARE_I2C0_BUS 0
#endif

#define IIC0_SCL_PIN TUYA_GPIO_NUM_30
#define IIC0_SDA_PIN TUYA_GPIO_NUM_31

#define TOF_I2C_ADDR 0x29

#define VL53L0X_REG_MODEL_ID                        0xC0
#define VL53L0X_REG_REVISION_ID                     0xC2
#define VL53L0X_REG_PRE_RANGE_CONFIG_VCSEL_PERIOD   0x50
#define VL53L0X_REG_FINAL_RANGE_CONFIG_VCSEL_PERIOD 0x70
#define VL53L0X_REG_SYSRANGE_START                  0x00
#define VL53L0X_REG_RESULT_RANGE_STATUS             0x14

#define APP_TOF_MODEL_ID_EXPECTED 0xEE

#define APP_TOF_POLL_MS                  200
/** Below this range (mm) is ignored; must be < APP_TOF_NEAR_THRESHOLD_MM for a valid near zone */
#define APP_TOF_IGNORE_BELOW_MM          30
/** Distance below this (mm) counts as near (cup present / close) */
#define APP_TOF_NEAR_THRESHOLD_MM        200

/** Same text as app_gpio_level_report.c when counting a drink */
#define APP_TOF_WATER_DRINK_AI_TEXT "我开始喝水了"

#define APP_TOF_IGNORE_ABOVE_MM          2000
#define APP_TOF_SYSRANGE_START_TIMEOUT   50
#define APP_TOF_RANGE_READY_MAX_ITER     100

/* ---------------------------------------------------------------------------
 * File scope variables
 * --------------------------------------------------------------------------- */
STATIC APP_TOF_CB_T   s_app_tof_cb     = NULL;
STATIC THREAD_HANDLE s_tof_thread_hdl = NULL;

/** Default handler: -1 unknown, 0 near band, 1 far band (avoid redundant lv_vendor_* / log spam) */
#define APP_TOF_BAND_UNKNOWN (-1)
#define APP_TOF_BAND_NEAR    0
#define APP_TOF_BAND_FAR   1
STATIC INT_T s_tof_default_last_band = APP_TOF_BAND_UNKNOWN;

/* ---------------------------------------------------------------------------
 * Forward declarations
 * --------------------------------------------------------------------------- */
STATIC VOID __tof_thread_process(VOID *arg);

/* ---------------------------------------------------------------------------
 * Function implementations
 * --------------------------------------------------------------------------- */
/**
 * @brief Decode VCSEL period register for debug logs
 * @param[in] val raw register byte
 * @return decoded period in PLL clocks
 */
STATIC UINT16_T __decode_vcsel_period(UINT8_T val)
{
    return (UINT16_T)((val + 1) << 1);
}

/**
 * @brief Read one register with bus lock
 * @param[in] reg register address
 * @return register value, 0xFF if transfer failed
 */
STATIC UINT8_T __tof_i2c_read_uint8(UINT8_T reg)
{
    UINT8_T       value = 0;
    OPERATE_RET   ret;
    const uint8_t regb = reg;

    app_i2c0_lock();
    ret = tkl_i2c_master_send(APP_TOF_I2C_PORT, TOF_I2C_ADDR, &regb, 1, TRUE);
    if (ret != OPRT_OK) {
        app_i2c0_unlock();
        return 0xFF;
    }
    ret = tkl_i2c_master_receive(APP_TOF_I2C_PORT, TOF_I2C_ADDR, &value, 1, FALSE);
    app_i2c0_unlock();
    if (ret != OPRT_OK) {
        return 0xFF;
    }
    return value;
}

/**
 * @brief Read consecutive registers from 0x14 result block
 * @param[in] reg start register
 * @param[out] buf output buffer
 * @param[in] len bytes to read
 * @return OPRT_OK on success
 */
STATIC OPERATE_RET __tof_i2c_read_block(UINT8_T reg, UINT8_T *buf, UINT16_T len)
{
    OPERATE_RET   ret;
    const uint8_t regb = reg;

    if (buf == NULL || len == 0) {
        return OPRT_INVALID_PARM;
    }

    app_i2c0_lock();
    ret = tkl_i2c_master_send(APP_TOF_I2C_PORT, TOF_I2C_ADDR, &regb, 1, TRUE);
    if (ret != OPRT_OK) {
        app_i2c0_unlock();
        return ret;
    }
    ret = tkl_i2c_master_receive(APP_TOF_I2C_PORT, TOF_I2C_ADDR, buf, len, FALSE);
    app_i2c0_unlock();
    return ret;
}

/**
 * @brief Write one register with bus lock
 * @param[in] reg register address
 * @param[in] value byte to write
 * @return transfer result
 */
STATIC OPERATE_RET __tof_i2c_write_uint8(UINT8_T reg, UINT8_T value)
{
    OPERATE_RET ret;
    uint8_t      data[2] = {reg, value};

    app_i2c0_lock();
    ret = tkl_i2c_master_send(APP_TOF_I2C_PORT, TOF_I2C_ADDR, data, 2, FALSE);
    app_i2c0_unlock();
    return ret;
}

/**
 * @brief Start single-shot range and wait for completion flag in SYSRANGE_START
 * @return OPRT_OK on success, OPRT_TIMEOUT on busy timeout
 */
STATIC OPERATE_RET __sysrange_start(VOID_T)
{
    INT_T       timeout = 0;
    OPERATE_RET wret;

    wret = __tof_i2c_write_uint8(VL53L0X_REG_SYSRANGE_START, 0x01);
    if (wret != OPRT_OK) {
        return wret;
    }

    while (__tof_i2c_read_uint8(VL53L0X_REG_SYSRANGE_START) & 0x01) {
        timeout++;
        tal_system_sleep(5);
        if (timeout > APP_TOF_SYSRANGE_START_TIMEOUT) {
            return OPRT_TIMEOUT;
        }
    }

    return OPRT_OK;
}

/**
 * @brief Poll RESULT_RANGE_STATUS for new sample ready
 * @return TRUE when bit0 set, FALSE on timeout
 */
STATIC BOOL_T __sysrange_is_ready(VOID_T)
{
    INT_T   cnt = 0;
    UINT8_T val = 0;

    while (cnt < APP_TOF_RANGE_READY_MAX_ITER) {
        tal_system_sleep(10);
        val = __tof_i2c_read_uint8(VL53L0X_REG_RESULT_RANGE_STATUS);
        if (val & 0x01) {
            return TRUE;
        }
        cnt++;
    }

    return FALSE;
}

/**
 * @brief Background ranging loop
 * @param[in] arg unused
 * @return none
 */
STATIC VOID __tof_thread_process(VOID *arg)
{
    UINT8_T  gbuf[12] = {0};
    UINT16_T dist     = 0;
    UINT8_T  status   = 0;

    (void)arg;

    while (1) {
        tal_system_sleep(APP_TOF_POLL_MS);

        if (__sysrange_start() != OPRT_OK) {
            continue;
        }

        if (!__sysrange_is_ready()) {
            continue;
        }

        if (__tof_i2c_read_block(0x14, gbuf, (UINT16_T)sizeof(gbuf)) != OPRT_OK) {
            continue;
        }

        dist   = (UINT16_T)MAKEWORD(gbuf[11], gbuf[10]);
        status = (UINT8_T)(((gbuf[0] & 0x78) >> 3));

        if (s_app_tof_cb != NULL) {
            s_app_tof_cb(dist);
        } else {
            PR_DEBUG("TOF mm:%u status:%u", (unsigned int)dist, (unsigned int)status);
        }
    }
}

/**
 * @brief Start VL53L0X background ranging thread
 * @param[in] cb optional user callback; may be NULL to only log distance
 * @return OPRT_OK on success, OPRT_COM_ERROR if sensor not detected
 */
OPERATE_RET app_tof_init(APP_TOF_CB_T cb)
{
    OPERATE_RET         rt = OPRT_OK;
    UINT8_T             val = 0;
    TUYA_IIC_BASE_CFG_T i2c_cfg = {.role = TUYA_IIC_MODE_MASTER,
                                 .speed = TUYA_IIC_BUS_SPEED_100K,
                                 .addr_width = TUYA_IIC_ADDRESS_7BIT};
    THREAD_CFG_T thrd_param = {0};

    thrd_param.thrdname   = "tof_monitor";
    thrd_param.priority = THREAD_PRIO_1;
    thrd_param.stackDepth = 2048;

    rt = app_i2c0_lock_init();
    if (rt != OPRT_OK) {
        PR_ERR("app_i2c0_lock_init failed: %d", rt);
        return rt;
    }

#if !(APP_TOF_SHARE_I2C0_BUS)
    PR_DEBUG("TOF: exclusive I2C0 init scl=%d sda=%d", IIC0_SCL_PIN, IIC0_SDA_PIN);
    tkl_io_pinmux_config(IIC0_SCL_PIN, TUYA_IIC0_SCL);
    tkl_io_pinmux_config(IIC0_SDA_PIN, TUYA_IIC0_SDA);
    rt = tkl_i2c_init(APP_TOF_I2C_PORT, &i2c_cfg);
    if (rt != OPRT_OK) {
        PR_ERR("tkl_i2c_init failed: %d", rt);
        return rt;
    }
#else
    (void)i2c_cfg;
    PR_DEBUG("TOF: using shared I2C0 (gesture must be initialized first)");
#endif

    s_app_tof_cb = cb;

    val = __tof_i2c_read_uint8(VL53L0X_REG_REVISION_ID);
    PR_DEBUG("TOF revision ID: 0x%02x", val);

    val = __tof_i2c_read_uint8(VL53L0X_REG_MODEL_ID);
    PR_DEBUG("TOF model ID: 0x%02x", val);
    if (val != APP_TOF_MODEL_ID_EXPECTED) {
        PR_ERR("TOF sensor not found (expected model 0x%02x)", APP_TOF_MODEL_ID_EXPECTED);
        goto err_exit;
    }

    val = __tof_i2c_read_uint8(VL53L0X_REG_PRE_RANGE_CONFIG_VCSEL_PERIOD);
    PR_DEBUG("TOF PRE_RANGE VCSEL: 0x%02x decode 0x%04x", val, __decode_vcsel_period(val));

    val = __tof_i2c_read_uint8(VL53L0X_REG_FINAL_RANGE_CONFIG_VCSEL_PERIOD);
    PR_DEBUG("TOF FINAL_RANGE VCSEL: 0x%02x decode 0x%04x", val, __decode_vcsel_period(val));

    rt = tal_thread_create_and_start(&s_tof_thread_hdl, NULL, NULL, __tof_thread_process, NULL, &thrd_param);
    if (rt != OPRT_OK) {
        PR_ERR("tal_thread_create_and_start tof failed: %d", rt);
        goto err_exit;
    }

    PR_INFO("TOF VL53L0X ranging started");
    return OPRT_OK;

err_exit:
#if !(APP_TOF_SHARE_I2C0_BUS)
    (void)tkl_i2c_deinit(APP_TOF_I2C_PORT);
#endif
    return OPRT_COM_ERROR;
}

/**
 * @brief Default proximity handler: PR_INFO distance; LVGL on band change; drink on near->far
 * @param[in] distance_mm range in millimeters
 * @return none
 * @note On NEAR->FAR, mirrors app_gpio_level_report.c: record drink, report DP, ai_agent_send_text.
 * @note Repeated lv_vendor_stop/start when already stopped/running causes PR_NOTICE spam in lv_vendor.c
 */
VOID app_tof_default_proximity_handler(UINT16_T distance_mm)
{
    INT_T band;
    INT_T prev_band;

    if (distance_mm < (UINT16_T)APP_TOF_IGNORE_BELOW_MM || distance_mm > (UINT16_T)APP_TOF_IGNORE_ABOVE_MM) {
        return;
    }

    band = (distance_mm < (UINT16_T)APP_TOF_NEAR_THRESHOLD_MM) ? APP_TOF_BAND_NEAR : APP_TOF_BAND_FAR;

    PR_INFO("TOF distance: %u mm (threshold %u mm, %s)", (unsigned int)distance_mm,
            (unsigned int)APP_TOF_NEAR_THRESHOLD_MM, (band == APP_TOF_BAND_NEAR) ? "near" : "far");

    if (band == s_tof_default_last_band) {
        return;
    }

    prev_band = s_tof_default_last_band;

    if (prev_band == APP_TOF_BAND_NEAR && band == APP_TOF_BAND_FAR) {
        int           today_total = 0;
        OPERATE_RET   drink_rt;
        OPERATE_RET   ai_rt;
        char          drink_msg[] = APP_TOF_WATER_DRINK_AI_TEXT;

        drink_rt = app_water_stats_record_drink(&today_total);
        if (drink_rt == OPRT_OK) {
            (void)app_water_stats_report_dp((uint32_t)today_total);
            ai_rt = ai_agent_send_text(drink_msg);
            if (ai_rt != OPRT_OK) {
                PR_DEBUG("tof: ai_agent_send_text ret=%d (agent may not be ready yet)", ai_rt);
            }
        } else {
            PR_WARN("tof: app_water_stats_record_drink failed: %d", drink_rt);
        }
    }

    s_tof_default_last_band = band;

    if (band == APP_TOF_BAND_NEAR) {
        lv_vendor_stop();
    } else {
        lv_vendor_start(5, 1024 * 8);
    }
}
