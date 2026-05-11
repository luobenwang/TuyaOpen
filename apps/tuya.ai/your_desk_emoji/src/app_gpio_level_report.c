/**
 * @file app_gpio_level_report.c
 * @brief Poll GPIO level and upload Tuya bool DP for pin state
 * @version 1.0
 * @date 2026-04-30
 * @copyright Copyright (c) 2026 Tuya Inc. All Rights Reserved.
 */

#include "app_gpio_level_report.h"

#include "app_water_stats.h"
#include "tuya_config.h"
#include "tal_api.h"
#include "tal_sw_timer.h"
#include "tkl_gpio.h"
#include "tkl_output.h"
#include "tuya_iot.h"
#include "tuya_iot_dp.h"

#include "ai_agent.h"

/* ---------------------------------------------------------------------------
 * Macros
 * --------------------------------------------------------------------------- */

#define APP_GPIO_LEVEL_REPORT_PIN TUYA_GPIO_NUM_2

#define APP_GPIO_LEVEL_REPORT_POLL_MS 50

/** User text pushed to AI agent when a drink is counted (GPIO rising edge) */
#define APP_WATER_DRINK_AI_TEXT "我开始喝水了"

/* ---------------------------------------------------------------------------
 * File scope variables
 * --------------------------------------------------------------------------- */

STATIC TIMER_ID           s_poll_timer     = NULL;
STATIC BOOL_T             s_has_last_level = FALSE;
STATIC TUYA_GPIO_LEVEL_E  s_last_level     = TUYA_GPIO_LEVEL_LOW;

/* ---------------------------------------------------------------------------
 * Function implementations
 * --------------------------------------------------------------------------- */

/**
 * @brief Map raw GPIO level to cloud bool (high -> true)
 * @param[in] level GPIO read result
 * @return true if physical high
 */
STATIC bool __gpio_level_to_bool(TUYA_GPIO_LEVEL_E level)
{
    return (level == TUYA_GPIO_LEVEL_HIGH) ? true : false;
}

/**
 * @brief Upload level as PROP_BOOL
 * @param[in] high true for high level, false for low
 * @return OPRT_OK on success
 */
STATIC OPERATE_RET __upload_level_bool(bool high)
{
    tuya_iot_client_t *client = tuya_iot_client_get();
    if (client == NULL) {
        PR_DEBUG("level_report: tuya client NULL, skip upload");
        return OPRT_COM_ERROR;
    }
    if (client->activate.devid == NULL) {
        PR_DEBUG("level_report: devid NULL, skip upload");
        return OPRT_COM_ERROR;
    }

    dp_obj_t dp        = {0};
    dp.id              = (uint8_t)APP_GPIO_LEVEL_REPORT_DPID;
    dp.type            = PROP_BOOL;
    dp.value.dp_bool   = high;
    dp.time_stamp      = 0;

    OPERATE_RET ret = tuya_iot_dp_obj_report(client, client->activate.devid, &dp, 1, 0);
    PR_DEBUG("level_report: DP bool id=%u val=%d ret=%d", (unsigned int)APP_GPIO_LEVEL_REPORT_DPID, high ? 1 : 0, ret);
    return ret;
}

/**
 * @brief Sample GPIO and report on change
 * @param[in] timer_id Software timer handle
 * @param[in] arg Unused callback context
 * @return none
 */
STATIC VOID_T __level_report_timer_cb(TIMER_ID timer_id, void *arg)
{
    (void)timer_id;
    (void)arg;

    TUYA_GPIO_LEVEL_E level = TUYA_GPIO_LEVEL_LOW;
    OPERATE_RET       ret   = tkl_gpio_read(APP_GPIO_LEVEL_REPORT_PIN, &level);
    if (ret != OPRT_OK) {
        PR_ERR("level_report: GPIO read fail pin=%d ret=%d", APP_GPIO_LEVEL_REPORT_PIN, ret);
        return;
    }

    if (s_has_last_level && (level == s_last_level)) {
        return;
    }

    if (s_has_last_level) {
        PR_NOTICE("level_report: edge pin=%d raw %d -> %d (bool %d -> %d)", APP_GPIO_LEVEL_REPORT_PIN,
                  (int)s_last_level, (int)level, __gpio_level_to_bool(s_last_level) ? 1 : 0,
                  __gpio_level_to_bool(level) ? 1 : 0);
    }

    if (s_has_last_level && (s_last_level == TUYA_GPIO_LEVEL_LOW) && (level == TUYA_GPIO_LEVEL_HIGH)) {
        int          today_total = 0;
        OPERATE_RET drink_rt;
        OPERATE_RET ai_rt;
        char          drink_msg[] = APP_WATER_DRINK_AI_TEXT;

        drink_rt = app_water_stats_record_drink(&today_total);
        if (drink_rt == OPRT_OK) {
            (void)app_water_stats_report_dp((uint32_t)today_total);
            ai_rt = ai_agent_send_text(drink_msg);
            if (ai_rt != OPRT_OK) {
                PR_DEBUG("level_report: ai_agent_send_text ret=%d (agent may not be ready yet)", ai_rt);
            }
        }
    }

    s_last_level     = level;
    s_has_last_level = TRUE;

    ret = __upload_level_bool(__gpio_level_to_bool(level));
    if (ret != OPRT_OK) {
        PR_WARN("level_report: upload after edge ret=%d", ret);
    }
}

/**
 * @brief Initialize GPIO as input and periodic sampling for level reporting
 * @return OPRT_OK on success
 */
OPERATE_RET app_gpio_level_report_init(void)
{
    TUYA_GPIO_BASE_CFG_T cfg = {
        .mode   = TUYA_GPIO_PULLUP,
        .direct = TUYA_GPIO_INPUT,
    };

    OPERATE_RET ret = tkl_gpio_init(APP_GPIO_LEVEL_REPORT_PIN, &cfg);
    if (ret != OPRT_OK) {
        PR_ERR("level_report: GPIO init fail pin=%d ret=%d", APP_GPIO_LEVEL_REPORT_PIN, ret);
        return ret;
    }

    ret = tal_sw_timer_create(__level_report_timer_cb, NULL, &s_poll_timer);
    if (ret != OPRT_OK) {
        PR_ERR("level_report: timer create fail:%d", ret);
        return ret;
    }

    ret = tal_sw_timer_start(s_poll_timer, APP_GPIO_LEVEL_REPORT_POLL_MS, TAL_TIMER_CYCLE);
    if (ret != OPRT_OK) {
        PR_ERR("level_report: timer start fail:%d", ret);
        (void)tal_sw_timer_delete(s_poll_timer);
        s_poll_timer = NULL;
        return ret;
    }

    return OPRT_OK;
}

/**
 * @brief Report current GPIO level to cloud
 * @return OPRT_OK on success
 */
OPERATE_RET app_gpio_level_report_sync(void)
{
    TUYA_GPIO_LEVEL_E level = TUYA_GPIO_LEVEL_LOW;
    OPERATE_RET       ret   = tkl_gpio_read(APP_GPIO_LEVEL_REPORT_PIN, &level);
    if (ret != OPRT_OK) {
        return ret;
    }

    s_last_level     = level;
    s_has_last_level = TRUE;

    return __upload_level_bool(__gpio_level_to_bool(level));
}
