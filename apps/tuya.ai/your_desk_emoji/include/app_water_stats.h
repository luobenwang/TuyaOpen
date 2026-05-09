/**
 * @file app_water_stats.h
 * @brief Daily drink counts persisted in KV (rolling 10 local days)
 * @version 1.0
 * @date 2026-04-30
 * @copyright Copyright (c) 2026 Tuya Inc. All Rights Reserved.
 */
#ifndef __APP_WATER_STATS_H__
#define __APP_WATER_STATS_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Load drink stats from flash (call before GPIO monitor)
 * @return OPRT_OK on success
 */
OPERATE_RET app_water_stats_init(void);

/**
 * @brief Record one drink for local calendar day (GPIO low-to-high)
 * @param[out] today_total_out Optional; today's total count after increment
 * @return OPRT_OK on success, OPRT_COM_ERROR if local date unavailable
 */
OPERATE_RET app_water_stats_record_drink(int *today_total_out);

/**
 * @brief Upload today's drink count as (PROP_VALUE)
 * @param[in] today_count Value to report for today
 * @return OPRT_OK on success
 */
OPERATE_RET app_water_stats_report_dp(uint32_t today_count);

/**
 * @brief On MQTT up: if today has at least one drink, report DP106 once
 * @return OPRT_OK or upload skipped / partial
 */
OPERATE_RET app_water_stats_sync_upload_today(void);

/**
 * @brief Query today's drink count without incrementing
 * @param[out] today_total_out Today's count, 0 if none or date invalid
 * @return OPRT_OK if date valid
 */
OPERATE_RET app_water_stats_get_today_count(int *today_total_out);

/**
 * @brief Print recent stored drink counts (KV, up to 10 days) to log
 * @return none
 */
VOID_T app_water_stats_print_recent(void);

/**
 * @brief Wipe all persisted drink stats (delete KV + clear in-memory cache)
 * @return OPRT_OK on success, KV error code otherwise
 * @note Intended to be called on factory-reset / device-removal events so the
 *       Water Baby ambient ladder restarts from level 0 for the next user.
 */
OPERATE_RET app_water_stats_clear(void);

#ifdef __cplusplus
}
#endif

#endif /* __APP_WATER_STATS_H__ */
