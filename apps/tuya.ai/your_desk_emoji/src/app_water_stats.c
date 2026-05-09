/**
 * @file app_water_stats.c
 * @brief Persist drink counts per calendar day (KV JSON, last 10 days)
 * @version 1.0
 * @date 2026-04-30
 * @copyright Copyright (c) 2026 Tuya Inc. All Rights Reserved.
 */

#include "app_water_stats.h"

#include "cJSON.h"
#include "tuya_config.h"
#include "tal_api.h"
#include "tal_time_service.h"
#include "tkl_output.h"
#include "tuya_iot.h"
#include "tuya_iot_dp.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---------------------------------------------------------------------------
 * Macros
 * --------------------------------------------------------------------------- */

#define WATER_STATS_KV_KEY "ty_desk_water_10d"

#define WATER_STATS_MAX_DAYS 10

/* ---------------------------------------------------------------------------
 * Type definitions
 * --------------------------------------------------------------------------- */

typedef struct {
    uint32_t date_id;
    uint32_t cnt;
} WATER_DAY_Entry_T;

/* ---------------------------------------------------------------------------
 * File scope variables
 * --------------------------------------------------------------------------- */

STATIC WATER_DAY_Entry_T s_days[WATER_STATS_MAX_DAYS + 1];
STATIC uint8_t           s_day_n = 0;

/* ---------------------------------------------------------------------------
 * Internal helpers
 * --------------------------------------------------------------------------- */

/**
 * @brief Compare two day entries by date_id (descending)
 * @param[in] a Pointer to first entry
 * @param[in] b Pointer to second entry
 * @return qsort comparison result
 */
STATIC int __water_day_cmp_desc(const void *a, const void *b)
{
    const WATER_DAY_Entry_T *da = (const WATER_DAY_Entry_T *)a;
    const WATER_DAY_Entry_T *db = (const WATER_DAY_Entry_T *)b;
    if (da->date_id > db->date_id) {
        return -1;
    }
    if (da->date_id < db->date_id) {
        return 1;
    }
    return 0;
}

/**
 * @brief Build YYYYMMDD from local wall clock
 * @param[out] out_id Calendar day id
 * @return OPRT_OK on success
 */
STATIC OPERATE_RET __get_local_date_id(uint32_t *out_id)
{
    POSIX_TM_S    tm;
    OPERATE_RET   rt = tal_time_get_local_time_custom(0, &tm);
    unsigned int y;
    unsigned int mo;
    unsigned int d;

    if (out_id == NULL) {
        return OPRT_INVALID_PARM;
    }
    if (rt != OPRT_OK) {
        return rt;
    }
    y  = (unsigned int)(tm.tm_year + 1900);
    mo = (unsigned int)(tm.tm_mon + 1);
    d  = (unsigned int)tm.tm_mday;
    *out_id = y * 10000u + mo * 100u + d;
    return OPRT_OK;
}

/**
 * @brief Serialize and write KV
 * @return OPRT_OK on success
 */
STATIC OPERATE_RET __water_stats_save(void)
{
    cJSON *root = cJSON_CreateObject();
    cJSON *arr  = NULL;
    char  *json_str = NULL;
    OPERATE_RET rt = OPRT_OK;

    if (root == NULL) {
        return OPRT_MALLOC_FAILED;
    }
    arr = cJSON_CreateArray();
    if (arr == NULL) {
        cJSON_Delete(root);
        return OPRT_MALLOC_FAILED;
    }

    for (uint8_t i = 0; i < s_day_n; i++) {
        cJSON *o = cJSON_CreateObject();
        if (o == NULL) {
            rt = OPRT_MALLOC_FAILED;
            break;
        }
        cJSON_AddNumberToObject(o, "date", (double)s_days[i].date_id);
        cJSON_AddNumberToObject(o, "cnt", (double)s_days[i].cnt);
        cJSON_AddItemToArray(arr, o);
    }

    if (rt != OPRT_OK) {
        cJSON_Delete(root);
        return rt;
    }

    cJSON_AddItemToObject(root, "days", arr);
    json_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    if (json_str == NULL) {
        return OPRT_MALLOC_FAILED;
    }

    rt = tal_kv_set(WATER_STATS_KV_KEY, (const uint8_t *)json_str, strlen(json_str));
    cJSON_free(json_str);
    PR_DEBUG("water_stats: saved KV %s", WATER_STATS_KV_KEY);
    return rt;
}

/**
 * @brief Parse KV into s_days
 * @return OPRT_OK on success
 */
STATIC OPERATE_RET __water_stats_load(void)
{
    uint8_t *raw = NULL;
    size_t   len = 0;

    s_day_n = 0;
    {
        int kvr = tal_kv_get(WATER_STATS_KV_KEY, &raw, &len);
        if (kvr != OPRT_OK || raw == NULL || len == 0) {
            if (raw != NULL) {
                (void)tal_kv_free(raw);
            }
            return OPRT_OK;
        }
    }

    cJSON *root = cJSON_Parse((char *)raw);
    tal_kv_free(raw);
    if (root == NULL) {
        return OPRT_OK;
    }

    cJSON *days = cJSON_GetObjectItem(root, "days");
    if (!cJSON_IsArray(days)) {
        cJSON_Delete(root);
        return OPRT_OK;
    }

    int n = cJSON_GetArraySize(days);
    for (int i = 0; i < n && s_day_n < WATER_STATS_MAX_DAYS; i++) {
        cJSON *o = cJSON_GetArrayItem(days, i);
        if (!cJSON_IsObject(o)) {
            continue;
        }
        cJSON *jd = cJSON_GetObjectItem(o, "date");
        cJSON *jc = cJSON_GetObjectItem(o, "cnt");
        if (!cJSON_IsNumber(jd) || !cJSON_IsNumber(jc)) {
            continue;
        }
        s_days[s_day_n].date_id = (uint32_t)jd->valuedouble;
        s_days[s_day_n].cnt     = (uint32_t)jc->valuedouble;
        if (s_days[s_day_n].cnt > 1000000u) {
            s_days[s_day_n].cnt = 1000000u;
        }
        s_day_n++;
    }

    cJSON_Delete(root);

    if (s_day_n > 1) {
        qsort(s_days, s_day_n, sizeof(WATER_DAY_Entry_T), __water_day_cmp_desc);
        if (s_day_n > WATER_STATS_MAX_DAYS) {
            s_day_n = WATER_STATS_MAX_DAYS;
        }
    }
    return OPRT_OK;
}

/**
 * @brief Today's count from loaded table
 * @param[in] today_id Calendar day id
 * @param[out] cnt_out Today's total
 * @return OPRT_OK if today row exists
 */
/**
 * @brief Format calendar day id YYYYMMDD to ASCII for logs
 * @param[in] date_id Calendar id
 * @param[out] buf Output buffer
 * @param[in] sz Buffer size
 * @return none
 */
STATIC VOID_T __water_format_date_id(uint32_t date_id, char *buf, size_t sz)
{
    unsigned int y;
    unsigned int m;
    unsigned int d;

    if (buf == NULL || sz == 0) {
        return;
    }
    y = date_id / 10000u;
    m = (date_id / 100u) % 100u;
    d = date_id % 100u;
    (void)snprintf(buf, sz, "%04u-%02u-%02u", y, m, d);
}

STATIC OPERATE_RET __find_today_count(uint32_t today_id, uint32_t *cnt_out)
{
    if (cnt_out == NULL) {
        return OPRT_INVALID_PARM;
    }
    *cnt_out = 0;
    for (uint8_t i = 0; i < s_day_n; i++) {
        if (s_days[i].date_id == today_id) {
            *cnt_out = s_days[i].cnt;
            return OPRT_OK;
        }
    }
    return OPRT_COM_ERROR;
}

/* ---------------------------------------------------------------------------
 * Public API
 * --------------------------------------------------------------------------- */

/**
 * @brief Load drink stats from flash (call before GPIO monitor)
 * @return OPRT_OK on success
 */
OPERATE_RET app_water_stats_init(void)
{
    OPERATE_RET rt;

    memset(s_days, 0, sizeof(s_days));
    s_day_n = 0;
    rt      = __water_stats_load();
    if (rt != OPRT_OK) {
        return rt;
    }

    PR_NOTICE("water_stats: init OK (startup snapshot)");
    app_water_stats_print_recent();

    return OPRT_OK;
}

/**
 * @brief Query today's drink count without incrementing
 * @param[out] today_total_out Today's count, 0 if none or date invalid
 * @return OPRT_OK if date valid
 */
OPERATE_RET app_water_stats_get_today_count(int *today_total_out)
{
    uint32_t today_id = 0;
    uint32_t cnt      = 0;
    OPERATE_RET rt;

    if (today_total_out == NULL) {
        return OPRT_INVALID_PARM;
    }
    *today_total_out = 0;

    rt = __get_local_date_id(&today_id);
    if (rt != OPRT_OK) {
        return rt;
    }
    if (__find_today_count(today_id, &cnt) == OPRT_OK) {
        *today_total_out = (int)cnt;
    }
    return OPRT_OK;
}

/**
 * @brief Record one drink for local calendar day (GPIO low-to-high)
 * @param[out] today_total_out Optional; today's total count after increment
 * @return OPRT_OK on success, OPRT_COM_ERROR if local date unavailable
 */
OPERATE_RET app_water_stats_record_drink(int *today_total_out)
{
    uint32_t    today_id = 0;
    int         idx      = -1;
    OPERATE_RET rt       = __get_local_date_id(&today_id);

    if (rt != OPRT_OK) {
        PR_WARN("water_stats: no local date, skip drink record");
        return OPRT_COM_ERROR;
    }

    for (uint8_t i = 0; i < s_day_n; i++) {
        if (s_days[i].date_id == today_id) {
            idx = (int)i;
            break;
        }
    }

    if (idx >= 0) {
        s_days[idx].cnt++;
    } else {
        if (s_day_n < WATER_STATS_MAX_DAYS) {
            s_days[s_day_n].date_id = today_id;
            s_days[s_day_n].cnt     = 1;
            s_day_n++;
        } else {
            int oldest = 0;
            for (uint8_t i = 1; i < s_day_n; i++) {
                if (s_days[i].date_id < s_days[oldest].date_id) {
                    oldest = (int)i;
                }
            }
            s_days[oldest].date_id = today_id;
            s_days[oldest].cnt     = 1;
        }
    }

    qsort(s_days, s_day_n, sizeof(WATER_DAY_Entry_T), __water_day_cmp_desc);
    if (s_day_n > WATER_STATS_MAX_DAYS) {
        s_day_n = WATER_STATS_MAX_DAYS;
    }

    rt = __water_stats_save();
    if (rt != OPRT_OK) {
        PR_WARN("water_stats: save failed %d", rt);
        return rt;
    }

    uint32_t total = 0;
    (void)__find_today_count(today_id, &total);
    if (today_total_out != NULL) {
        *today_total_out = (total > (uint32_t)INT_MAX) ? INT_MAX : (int)total;
    }

    PR_NOTICE("water_stats: drink date=%u today_total=%u", (unsigned int)today_id, (unsigned int)total);
    return OPRT_OK;
}

/**
 * @brief Upload today's drink count as DP106 (PROP_VALUE)
 * @param[in] today_count Value to report for today
 * @return OPRT_OK on success
 */
OPERATE_RET app_water_stats_report_dp(uint32_t today_count)
{
    tuya_iot_client_t *client = tuya_iot_client_get();
    int                  val;

    if (client == NULL || client->activate.devid == NULL) {
        PR_WARN("water_stats: client/devid NULL, skip DP106");
        return OPRT_COM_ERROR;
    }

    val = (today_count > (uint32_t)INT_MAX) ? INT_MAX : (int)today_count;

    dp_obj_t dp       = {0};
    dp.id             = (uint8_t)APP_WATER_TIMES_REPORT_DPID;
    dp.type           = PROP_VALUE;
    dp.value.dp_value = val;
    dp.time_stamp     = 0;

    OPERATE_RET ret =
        tuya_iot_dp_obj_report(client, client->activate.devid, &dp, 1, 0);
    PR_NOTICE("water_stats: DP106 VALUE=%d ret=%d", val, ret);
    return ret;
}

/**
 * @brief On MQTT up: if today has at least one drink, report DP106 once
 * @return OPRT_OK or upload skipped / partial
 */
OPERATE_RET app_water_stats_sync_upload_today(void)
{
    int         today_cnt = 0;
    OPERATE_RET rt        = app_water_stats_get_today_count(&today_cnt);

    if (rt != OPRT_OK) {
        return rt;
    }
    if (today_cnt <= 0) {
        return OPRT_OK;
    }
    return app_water_stats_report_dp((uint32_t)today_cnt);
}

/**
 * @brief Wipe all persisted drink stats (delete KV + clear in-memory cache)
 * @return OPRT_OK on success, KV error code otherwise
 * @note Idempotent. Safe to call when no record exists. Intended for
 *       factory-reset / device-removal flows.
 */
OPERATE_RET app_water_stats_clear(void)
{
    OPERATE_RET rt;

    memset(s_days, 0, sizeof(s_days));
    s_day_n = 0;

    rt = tal_kv_del(WATER_STATS_KV_KEY);
    if (rt != OPRT_OK) {
        PR_WARN("water_stats: KV del '%s' rt=%d (treated as cleared)", WATER_STATS_KV_KEY, rt);
    } else {
        PR_NOTICE("water_stats: KV '%s' cleared", WATER_STATS_KV_KEY);
    }
    return rt;
}

/**
 * @brief Print recent stored drink counts (KV, up to 10 days) to log
 * @return none
 */
VOID_T app_water_stats_print_recent(void)
{
    char date_buf[16];

    PR_NOTICE("water_stats: recent drinks (newest first, max %u days):", (unsigned int)WATER_STATS_MAX_DAYS);

    if (s_day_n == 0) {
        PR_NOTICE("water_stats:   (no records)");
    } else {
        for (uint8_t i = 0; i < s_day_n; i++) {
            __water_format_date_id(s_days[i].date_id, date_buf, sizeof(date_buf));
            PR_NOTICE("water_stats:   date=%s count=%u", date_buf, (unsigned int)s_days[i].cnt);
        }
    }

    {
        int today_cnt = 0;

        if (app_water_stats_get_today_count(&today_cnt) == OPRT_OK) {
            PR_NOTICE("water_stats:   today_total=%d (local calendar)", today_cnt);
        } else {
            PR_NOTICE("water_stats:   today_total=n/a (local time not ready)");
        }
    }
}
