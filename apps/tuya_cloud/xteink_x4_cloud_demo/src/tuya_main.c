/**
 * @file tuya_main.c
 * @brief XTEINK X4 demo entry (optional LVGL EPD UI + optional Tuya cloud).
 * @version 1.0
 * @date 2026-07-08
 * @copyright Copyright (c) 2026 Tuya Inc. All Rights Reserved.
 */
#include "tal_api.h"
#include "tal_system.h"
#include "tkl_output.h"
#include "tuya_config.h"
#include <string.h>

#ifndef PROJECT_VERSION
#define PROJECT_VERSION "1.0.0"
#endif

#if XTEINK_X4_ENABLE_DISPLAY
#include "xteink_x4_display.h"
#endif

#if XTEINK_X4_ENABLE_CLOUD
#include "board_com_api.h"
#include "cJSON.h"
#include "netmgr.h"
#include "tuya_iot.h"
#include "tuya_iot_dp.h"
#include "tal_cli.h"
#include "tuya_authorize.h"
#include "reset_netcfg.h"

#define X4_PWR_HOLD_MS 3000U
#define X4_DISPLAY_READY_WAIT_MS 15000U

#if defined(ENABLE_WIFI) && (ENABLE_WIFI == 1)
#include "netconn_wifi.h"
#endif
#if defined(ENABLE_BLUETOOTH) && (ENABLE_BLUETOOTH == 1)
#include "ble_mgr.h"
#endif
#if defined(ENABLE_WIRED) && (ENABLE_WIRED == 1)
#include "netconn_wired.h"
#endif
#if defined(ENABLE_CELLULAR) && (ENABLE_CELLULAR == 1)
#include "netconn_cellular.h"
#endif
#if defined(ENABLE_LIBLWIP) && (ENABLE_LIBLWIP == 1)
#include "lwip_init.h"
#endif

#if defined(ENABLE_QRCODE) && (ENABLE_QRCODE == 1)
#include "qrencode_print.h"
#endif

extern void tuya_app_cli_init(void);

tuya_iot_client_t  client;
tuya_iot_license_t license;

/**
 * @brief Log free heap for cloud bring-up debug.
 * @param[in] tag checkpoint label
 * @return none
 */
static void __log_heap(const char *tag)
{
    PR_NOTICE("[heap] %s: free=%d", tag, tal_system_get_free_heap_size());
}

#if defined(ENABLE_BLUETOOTH) && (ENABLE_BLUETOOTH == 1)
/** Set on BIND_TOKEN_ON; BLE freed once WiFi link is up. */
static volatile BOOL_T s_x4_ble_release_pending = FALSE;

/**
 * @brief Deferred BLE teardown after WiFi is up (work queue context).
 * @param[in] arg unused
 * @return none
 * @note Must not run while STA mode is first enabling; that races coexist and
 *       panics with Load access fault on ESP32-C3.
 */
static void __x4_ble_deinit_on_workq(void *arg)
{
    (void)arg;
    (void)tuya_ble_deinit();
    __log_heap("after ble deinit (wifi up)");
}

/**
 * @brief On WiFi link-up after token, free BLE RAM for TLS/activate.
 * @param[in] data pointer to netmgr_status_e
 * @return OPRT_OK
 */
static OPERATE_RET __x4_on_link_status(void *data)
{
    netmgr_status_e status;

    if (FALSE == s_x4_ble_release_pending || NULL == data) {
        return OPRT_OK;
    }
    status = *(netmgr_status_e *)data;
    if (NETMGR_LINK_UP != status && NETMGR_LINK_UP_SWITH != status) {
        return OPRT_OK;
    }
    s_x4_ble_release_pending = FALSE;
    (void)tal_workq_schedule(WORKQ_SYSTEM, __x4_ble_deinit_on_workq, NULL);
    return OPRT_OK;
}
#endif

#if !XTEINK_X4_ENABLE_DISPLAY
/**
 * @brief Report cloud status on serial when LVGL UI is disabled.
 * @param[in] msg status text
 * @return none
 */
#define x4_cloud_status(msg) PR_NOTICE("[cloud-ui] %s", (msg))
#else
/**
 * @brief Report cloud status on EPD dashboard.
 * @param[in] msg status text
 * @return none
 */
#define x4_cloud_status(msg) xteink_x4_display_set_cloud_status(msg)
#endif

/**
 * @brief X4 board bring-up before WiFi/BLE (same policy as lvgl_demo).
 * @return none
 * @note When display+cloud, only wake gate runs here; EPD init is in display thread before WiFi.
 */
static void __x4_board_bootstrap(void)
{
    OPERATE_RET rt = OPRT_OK;

#if XTEINK_X4_ENABLE_DISPLAY && XTEINK_X4_ENABLE_CLOUD
    PR_NOTICE("X4 cloud bootstrap: buttons + wake gate (EPD before WiFi/BLE)");
    TUYA_CALL_ERR_LOG(board_x4_buttons_init());
#if X4_ENABLE_POWER_WAKE_GATE
    {
        X4_WAKEUP_CLASS_E cls;

        if (OPRT_OK == board_x4_sleep_classify_wakeup(&cls)) {
            PR_NOTICE("X4 wake class: %d", (int)cls);
            if (X4_WAKEUP_CLASS_AFTER_USB_POWER == cls) {
                PR_NOTICE("X4: USB-only power boot -> deep sleep (CrossPoint policy)");
                (void)board_x4_power_shutdown();
            } else if (X4_WAKEUP_CLASS_POWER_BUTTON == cls) {
                rt = board_x4_power_verify_gpio_wake((uint32_t)X4_PWR_HOLD_MS, FALSE);
                TUYA_CALL_ERR_LOG(rt);
            }
        }
    }
#endif
#elif XTEINK_X4_ENABLE_DISPLAY
    PR_NOTICE("X4 cloud bootstrap: deferred to display thread");
#elif X4_ENABLE_POWER_WAKE_GATE
    X4_WAKEUP_CLASS_E cls;

    PR_NOTICE("X4 cloud bootstrap: board_register_hardware (power gate on)");
    TUYA_CALL_ERR_LOG(board_register_hardware());

    if (OPRT_OK == board_x4_sleep_classify_wakeup(&cls)) {
        PR_NOTICE("X4 wake class: %d", (int)cls);
        if (X4_WAKEUP_CLASS_AFTER_USB_POWER == cls) {
            PR_NOTICE("X4: USB-only power boot -> deep sleep (CrossPoint policy)");
            (void)board_x4_power_shutdown();
        } else if (X4_WAKEUP_CLASS_POWER_BUTTON == cls) {
            rt = board_x4_power_verify_gpio_wake((uint32_t)X4_PWR_HOLD_MS, FALSE);
            TUYA_CALL_ERR_LOG(rt);
        }
    }

    (void)board_x4_epd_sleep();
    PR_NOTICE("X4 cloud bootstrap done (EPD sleep)");
#else
    PR_NOTICE("X4 cloud bootstrap: skipped (power gate off, USB/cloud debug OK)");
#endif
}

/**
 * @brief user defined log output api
 * @param[in] str log string
 * @return none
 */
void user_log_output_cb(const char *str)
{
    tkl_log_output(str);
}

/**
 * @brief OTA upgrade notify callback
 * @param[in] client device client
 * @param[in] upgrade upgrade JSON
 * @return none
 */
void user_upgrade_notify_on(tuya_iot_client_t *client, cJSON *upgrade)
{
    PR_INFO("----- Upgrade information -----");
    if (!upgrade) {
        PR_WARN("upgrade JSON is NULL");
        return;
    }

    cJSON *type_item    = cJSON_GetObjectItem(upgrade, "type");
    cJSON *version_item = cJSON_GetObjectItem(upgrade, "version");
    cJSON *size_item    = cJSON_GetObjectItem(upgrade, "size");
    cJSON *md5_item     = cJSON_GetObjectItem(upgrade, "md5");
    cJSON *hmac_item    = cJSON_GetObjectItem(upgrade, "hmac");
    cJSON *url_item     = cJSON_GetObjectItem(upgrade, "url");
    cJSON *https_item   = cJSON_GetObjectItem(upgrade, "httpsUrl");

    PR_INFO("OTA Channel: %d", cJSON_IsNumber(type_item) ? type_item->valueint : -1);
    PR_INFO("Version: %s", cJSON_IsString(version_item) ? version_item->valuestring : "N/A");
    PR_INFO("Size: %s", cJSON_IsString(size_item) ? size_item->valuestring : "N/A");
    PR_INFO("MD5: %s", cJSON_IsString(md5_item) ? md5_item->valuestring : "N/A");
    PR_INFO("HMAC: %s", cJSON_IsString(hmac_item) ? hmac_item->valuestring : "N/A");
    PR_INFO("URL: %s", cJSON_IsString(url_item) ? url_item->valuestring : "N/A");
    PR_INFO("HTTPS URL: %s", cJSON_IsString(https_item) ? https_item->valuestring : "N/A");

    x4_cloud_status("OTA available");
}

/**
 * @brief Tuya IoT event handler
 * @param[in] client device client
 * @param[in] event event message
 * @return none
 */
void user_event_handler_on(tuya_iot_client_t *client, tuya_event_msg_t *event)
{
    PR_DEBUG("Tuya Event ID:%d(%s)", event->id, EVENT_ID2STR(event->id));
    PR_INFO("Device Free heap %d", tal_system_get_free_heap_size());

    switch (event->id) {
    case TUYA_EVENT_BIND_START:
        PR_INFO("Device Bind Start!");
        x4_cloud_status("Binding...");
        break;

    case TUYA_EVENT_BIND_TOKEN_ON:
        PR_INFO("Bind token received");
        x4_cloud_status("Token OK, connecting...");
#if defined(ENABLE_BLUETOOTH) && (ENABLE_BLUETOOTH == 1)
        /* Defer BLE free until WiFi LINK_UP (see __x4_on_link_status). */
        s_x4_ble_release_pending = TRUE;
#endif
        break;

    case TUYA_EVENT_DIRECT_MQTT_CONNECTED: {
        x4_cloud_status("MQTT connected");
#if defined(ENABLE_QRCODE) && (ENABLE_QRCODE == 1)
        char buffer[255];
        snprintf(buffer, sizeof(buffer), "https://smartapp.tuya.com/s/p?p=%s&uuid=%s&v=2.0", TUYA_PRODUCT_ID,
                 license.uuid);
        qrcode_string_output(buffer, user_log_output_cb, 0);
#endif
    } break;

    case TUYA_EVENT_UPGRADE_NOTIFY:
        user_upgrade_notify_on(client, event->value.asJSON);
        break;

    case TUYA_EVENT_RESET: {
        tuya_reset_type_t reset_type = (tuya_reset_type_t)event->value.asInteger;
        PR_INFO("Device Reset:%d", reset_type);
        x4_cloud_status("Device reset");
    } break;

    case TUYA_EVENT_RESET_COMPLETE: {
        PR_INFO("Device Reset Complete!");
        x4_cloud_status("Rebooting...");
        tal_system_reset();
    } break;

    case TUYA_EVENT_DP_RECEIVE_OBJ: {
        dp_obj_recv_t *dpobj = event->value.dpobj;
        PR_DEBUG("SOC Rev DP Cmd t1:%d t2:%d CNT:%u", dpobj->cmd_tp, dpobj->dtt_tp, dpobj->dpscnt);
        if (dpobj->devid != NULL) {
            PR_DEBUG("devid.%s", dpobj->devid);
        }

        uint32_t index = 0;
        for (index = 0; index < dpobj->dpscnt; index++) {
            dp_obj_t *dp = dpobj->dps + index;
            PR_DEBUG("idx:%d dpid:%d type:%d ts:%u", index, dp->id, dp->type, dp->time_stamp);
            switch (dp->type) {
            case PROP_BOOL: {
                PR_DEBUG("bool value:%d", dp->value.dp_bool);
                break;
            }
            case PROP_VALUE: {
                PR_DEBUG("int value:%d", dp->value.dp_value);
                break;
            }
            case PROP_STR: {
                PR_DEBUG("str value:%s", dp->value.dp_str);
                break;
            }
            case PROP_ENUM: {
                PR_DEBUG("enum value:%u", dp->value.dp_enum);
                break;
            }
            case PROP_BITMAP: {
                PR_DEBUG("bits value:0x%X", dp->value.dp_bitmap);
                break;
            }
            default: {
                PR_ERR("idx:%d dpid:%d type:%d ts:%u is invalid", index, dp->id, dp->type, dp->time_stamp);
                break;
            }
            }
        }

        tuya_iot_dp_obj_report(client, dpobj->devid, dpobj->dps, dpobj->dpscnt, 0);
        x4_cloud_status("DP received");
    } break;

    case TUYA_EVENT_DP_RECEIVE_RAW: {
        dp_raw_recv_t *dpraw = event->value.dpraw;
        PR_DEBUG("SOC Rev DP Cmd t1:%d t2:%d", dpraw->cmd_tp, dpraw->dtt_tp);
        if (dpraw->devid != NULL) {
            PR_DEBUG("devid.%s", dpraw->devid);
        }

        uint32_t  index = 0;
        dp_raw_t *dp    = &dpraw->dp;
        PR_DEBUG("dpid:%d type:RAW len:%d data:", dp->id, dp->len);
        for (index = 0; index < dp->len; index++) {
            PR_DEBUG_RAW("%02x", dp->data[index]);
        }

        tuya_iot_dp_raw_report(client, dpraw->devid, &dpraw->dp, 3);
        x4_cloud_status("RAW DP received");
    } break;

    default:
        break;
    }
}

/**
 * @brief Network link check for Tuya IoT stack
 * @return true if network is up
 */
bool user_network_check(void)
{
    netmgr_status_e status = NETMGR_LINK_DOWN;
    netmgr_conn_get(NETCONN_AUTO, NETCONN_CMD_STATUS, &status);
    return status == NETMGR_LINK_DOWN ? false : true;
}

/**
 * @brief Run Tuya cloud stack (switch_demo flow).
 * @return none
 */
static void __user_main_cloud(void)
{
    int rt = OPRT_OK;

    cJSON_InitHooks(&(cJSON_Hooks){.malloc_fn = tal_malloc, .free_fn = tal_free});
#if XTEINK_X4_ENABLE_DISPLAY && XTEINK_X4_ENABLE_CLOUD
    tal_log_init(TAL_LOG_LEVEL_DEBUG, 1024, (TAL_LOG_OUTPUT_CB)tkl_log_output);
#else
    tal_log_init(TAL_LOG_LEVEL_DEBUG, 4096, (TAL_LOG_OUTPUT_CB)tkl_log_output);
#endif

    PR_NOTICE("Application information:");
    PR_NOTICE("Project name:        %s", PROJECT_NAME);
    PR_NOTICE("App version:         %s", PROJECT_VERSION);
    PR_NOTICE("Platform board:      %s", PLATFORM_BOARD);
    PR_NOTICE("Build mode:          cloud");
    __log_heap("boot");

    __x4_board_bootstrap();
    __log_heap("after board bootstrap");

    tal_kv_init(&(tal_kv_cfg_t){
        .seed = "vmlkasdh93dlvlcy",
        .key  = "dflfuap134ddlduq",
    });
    tal_sw_timer_init();
    tal_workq_init();
    __log_heap("after tal init");

#if !defined(PLATFORM_UBUNTU) || (PLATFORM_UBUNTU == 0)
#if defined(X4_USB_SERIAL_JTAG_CONSOLE) && (X4_USB_SERIAL_JTAG_CONSOLE == 1)
    PR_NOTICE("skip tal_cli_init (USB Serial JTAG on GPIO18/19)");
#else
    tal_cli_init();
#if defined(ENABLE_SERIAL_CLI_CMD) && (ENABLE_SERIAL_CLI_CMD == 1)
    tuya_app_cli_init();
#endif
#endif
    tuya_authorize_init();
#endif

    reset_netconfig_start();

    if (OPRT_OK != tuya_authorize_read(&license)) {
        license.uuid    = TUYA_OPENSDK_UUID;
        license.authkey = TUYA_OPENSDK_AUTHKEY;
        PR_WARN("Using TUYA_OPENSDK_UUID/AUTHKEY from tuya_config.h");
    }
    PR_NOTICE("license uuid: %s", license.uuid);

    rt = tuya_iot_init(&client, &(const tuya_iot_config_t){
                                    .software_ver  = PROJECT_VERSION,
                                    .productkey    = TUYA_PRODUCT_ID,
                                    .uuid          = license.uuid,
                                    .authkey       = license.authkey,
                                    .event_handler = user_event_handler_on,
                                    .network_check = user_network_check,
                                });
    if (OPRT_OK != rt) {
        PR_ERR("tuya_iot_init failed: %d", rt);
        for (;;) {
            tal_system_sleep(1000);
        }
    }
    __log_heap("after tuya_iot_init");

#if XTEINK_X4_ENABLE_DISPLAY
    TUYA_CALL_ERR_LOG(xteink_x4_display_start());
    x4_cloud_status("Cloud init...");
    __log_heap("after display start");
#if XTEINK_X4_ENABLE_CLOUD
    rt = xteink_x4_display_wait_ready(X4_DISPLAY_READY_WAIT_MS);
    if (OPRT_OK != rt) {
        PR_WARN("X4 display wait ready: %d (continuing)", rt);
    } else {
        PR_NOTICE("X4 display ready before WiFi/BLE");
    }
    __log_heap("after display ready");
#endif
#endif

#if defined(ENABLE_LIBLWIP) && (ENABLE_LIBLWIP == 1)
    TUYA_LwIP_Init();
#endif

    netmgr_type_e type = 0;
#if defined(ENABLE_WIFI) && (ENABLE_WIFI == 1)
    type |= NETCONN_WIFI;
#endif
#if defined(ENABLE_WIRED) && (ENABLE_WIRED == 1)
    type |= NETCONN_WIRED;
#endif
#if defined(ENABLE_CELLULAR) && (ENABLE_CELLULAR == 1)
    type |= NETCONN_CELLULAR;
#endif
    netmgr_init(type);
    __log_heap("after netmgr_init");

#if defined(ENABLE_BLUETOOTH) && (ENABLE_BLUETOOTH == 1)
    /* Free BLE only after STA is linked — not during wifi:enable tsf. */
    (void)tal_event_subscribe(EVENT_LINK_STATUS_CHG, "x4_ble_free", __x4_on_link_status, SUBSCRIBE_TYPE_NORMAL);
#endif

#if defined(ENABLE_WIFI) && (ENABLE_WIFI == 1)
#if XTEINK_X4_ENABLE_DISPLAY && XTEINK_X4_ENABLE_CLOUD
    netmgr_conn_set(NETCONN_WIFI, NETCONN_CMD_NETCFG, &(netcfg_args_t){.type = NETCFG_TUYA_BLE});
    x4_cloud_status("Netcfg BLE");
#else
    netmgr_conn_set(NETCONN_WIFI, NETCONN_CMD_NETCFG, &(netcfg_args_t){.type = NETCFG_TUYA_BLE | NETCFG_TUYA_WIFI_AP});
    x4_cloud_status("Netcfg BLE/AP");
#endif
#endif

    PR_NOTICE("tuya_iot_init success, starting cloud...");
    rt = tuya_iot_start(&client);
    if (OPRT_OK != rt) {
        PR_ERR("tuya_iot_start failed: %d", rt);
        for (;;) {
            tal_system_sleep(1000);
        }
    }
    x4_cloud_status("Cloud starting...");
    __log_heap("after tuya_iot_start");

    reset_netconfig_check();

    PR_NOTICE("enter tuya_iot_yield loop");
    {
        uint32_t hb_ms = 0;

        for (;;) {
            tuya_iot_yield(&client);

            uint32_t now = tal_system_get_millisecond();
            if ((now - hb_ms) >= 10000U) {
                hb_ms = now;
                __log_heap("yield heartbeat");
                x4_cloud_status("running");
            }
        }
    }
}
#endif /* XTEINK_X4_ENABLE_CLOUD */

#if XTEINK_X4_ENABLE_DISPLAY
/**
 * @brief Run LVGL/EPD UI only (same flow as lvgl_demo).
 * @return none
 */
static void __user_main_lvgl(void)
{
    OPERATE_RET rt = OPRT_OK;

    tal_log_init(TAL_LOG_LEVEL_DEBUG, 4096, (TAL_LOG_OUTPUT_CB)tkl_log_output);

    PR_NOTICE("xteink_x4 lvgl-only mode");
    PR_NOTICE("Project name:        %s", PROJECT_NAME);
    PR_NOTICE("Platform board:      %s", PLATFORM_BOARD);

    TUYA_CALL_ERR_LOG(xteink_x4_display_start());
    (void)rt;

    for (;;) {
        tal_system_sleep(1000);
    }
}
#endif /* XTEINK_X4_ENABLE_DISPLAY */

/**
 * @brief Application entry
 * @return none
 */
void user_main(void)
{
#if (OPERATING_SYSTEM != SYSTEM_LINUX) && (X4_BOOT_DEBUG_DELAY_MS > 0U)
    /* Debug: wait for USB Serial JTAG COM to enumerate after reset. */
    tal_system_sleep(X4_BOOT_DEBUG_DELAY_MS);
#endif
#if !XTEINK_X4_ENABLE_DISPLAY && XTEINK_X4_ENABLE_CLOUD
    __user_main_cloud();
#elif XTEINK_X4_ENABLE_DISPLAY && !XTEINK_X4_ENABLE_CLOUD
    __user_main_lvgl();
#elif XTEINK_X4_ENABLE_DISPLAY && XTEINK_X4_ENABLE_CLOUD
    __user_main_cloud();
#else
#error "Enable display and/or cloud in tuya_config.h"
#endif
}

#if OPERATING_SYSTEM == SYSTEM_LINUX
void main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    user_main();
}
#else

static THREAD_HANDLE ty_app_thread = NULL;

/**
 * @brief Application thread wrapper
 * @param[in] arg unused
 * @return none
 */
static void tuya_app_thread(void *arg)
{
    (void)arg;
    user_main();
    tal_thread_delete(ty_app_thread);
    ty_app_thread = NULL;
}

/**
 * @brief TuyaOpen application entry
 * @return none
 */
void tuya_app_main(void)
{
    static char  app_thread_name[] = "tuya_app_main";
    THREAD_CFG_T thrd_param;

    (void)memset(&thrd_param, 0, sizeof(thrd_param));
#if !XTEINK_X4_ENABLE_CLOUD
    thrd_param.stackDepth = 1024 * 4;
#else
    thrd_param.stackDepth = 1024 * 8;
#endif
    thrd_param.priority   = THREAD_PRIO_1;
    thrd_param.thrdname   = app_thread_name;

    tal_thread_create_and_start(&ty_app_thread, NULL, NULL, tuya_app_thread, NULL, &thrd_param);
}
#endif
