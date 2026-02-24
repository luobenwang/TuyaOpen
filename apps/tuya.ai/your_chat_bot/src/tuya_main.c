/**
 * @file tuya_main.c
 * @brief Implements main audio functionality for IoT device
 *
 * This source file provides the implementation of the main audio functionalities
 * required for an IoT device. It includes functionality for audio processing,
 * device initialization, event handling, and network communication. The
 * implementation supports audio volume control, data point processing, and
 * interaction with the Tuya IoT platform. This file is essential for developers
 * working on IoT applications that require audio capabilities and integration
 * with the Tuya IoT ecosystem.
 *
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 *
 */

#include "tuya_cloud_types.h"

#include <assert.h>
#include <string.h>
#include "cJSON.h"
#include "tal_api.h"
#include "tuya_config.h"
#include "tuya_iot.h"
#include "tuya_iot_dp.h"
#include "netmgr.h"
#include "tkl_output.h"
#include "tal_cli.h"
#include "tuya_authorize.h"
#if defined(ENABLE_WIFI) && (ENABLE_WIFI == 1)
#include "netconn_wifi.h"
#else
// Stub WiFi functions for non-WiFi platforms (e.g., Ubuntu with wired)
#include "tkl_wifi_stub.h"
#endif
#if defined(ENABLE_WIRED) && (ENABLE_WIRED == 1)
#include "netconn_wired.h"
#endif
#if defined(ENABLE_LIBLWIP) && (ENABLE_LIBLWIP == 1)
#include "lwip_init.h"
#endif

#if defined(ENABLE_CHAT_DISPLAY) && (ENABLE_CHAT_DISPLAY == 1)
#include "app_display.h"
#endif

#include "board_com_api.h"

#include "app_chat_bot.h"
#include "ai_audio.h"
#include "reset_netcfg.h"
#include "app_system_info.h"

#if defined(ENABLE_QRCODE) && (ENABLE_QRCODE == 1)
#include "qrencode_print.h"
#endif

/* Additional MQTT client includes for AI MQTT */
#include "mqtt_client_interface.h"
#include "tuya_config_defaults.h"

/* Tuya device handle */
tuya_iot_client_t ai_client;

/* Tuya license information (uuid authkey) */
tuya_iot_license_t license;

#ifndef PROJECT_VERSION
#define PROJECT_VERSION "1.0.0"
#endif

#define DPID_VOLUME 3

static uint8_t _need_reset = 0;

/***********************************************************
*********************** Additional MQTT Client ***********************
***********************************************************/
/* AI MQTT topics */
#define AI_CMD_TOPIC "AI_CMD"
#define AI_RET_TOPIC "AI_RET"

/* AI message callback function type */
typedef void (*ai_ret_message_cb_t)(const uint8_t *payload, size_t length, void *userdata);

/* Additional MQTT client variables */
static void *g_additional_mqtt_client_ctx = NULL;
static ai_ret_message_cb_t g_ai_ret_callback = NULL;
static void *g_ai_ret_userdata = NULL;
static uint16_t g_ai_ret_subscribe_msgid = 0;  // Track AI_RET subscription msgid
static bool g_additional_mqtt_connected = false;

/**
 * @brief user defined log output api, in this demo, it will use uart0 as log-tx
 *
 * @param str log string
 * @return void
 */
void user_log_output_cb(const char *str)
{
    tal_uart_write(TUYA_UART_NUM_0, (const uint8_t *)str, strlen(str));
}

/***********************************************************
*********************** Additional MQTT Client Functions ***********************
***********************************************************/

/* Forward declarations */
uint16_t ai_cmd_send(const uint8_t *data, size_t length, uint8_t qos);
int ai_ret_register_callback(ai_ret_message_cb_t callback, void *userdata);
void ai_ret_unregister_callback(void);

/**
 * @brief Additional MQTT client connected callback
 */
static void additional_mqtt_client_connected_cb(void *client, void *userdata)
{
    PR_INFO("Additional MQTT client connected! try to subscribe AI_RET topic");
    g_additional_mqtt_connected = true;
    
    /* Subscribe to AI_RET topic to receive AI responses */
    uint16_t msgid = mqtt_client_subscribe(client, AI_RET_TOPIC, MQTT_QOS_1);
    if (msgid > 0) {
        g_ai_ret_subscribe_msgid = msgid;
        PR_INFO("Subscribe AI_RET topic success, ID:%d", msgid);
    } else {
        PR_ERR("Subscribe AI_RET topic failed!");
    }
}

/**
 * @brief Additional MQTT client disconnected callback
 */
static void additional_mqtt_client_disconnected_cb(void *client, void *userdata)
{
    PR_INFO("Additional MQTT client disconnected!");
    g_additional_mqtt_connected = false;
}

/**
 * @brief Additional MQTT client message callback
 */
static void additional_mqtt_client_message_cb(void *client, uint16_t msgid, const mqtt_client_message_t *msg, void *userdata)
{
    PR_DEBUG("Additional MQTT recv message TopicName:%s, payload len:%d", msg->topic, msg->length);
    
    /* Handle AI_RET topic messages */
    if (msg->topic != NULL && strcmp(msg->topic, AI_RET_TOPIC) == 0) {
        PR_INFO("Received AI_RET message, length: %zu", msg->length);
        
        /* Print payload content as string */
        if (msg->payload != NULL && msg->length > 0) {
            PR_INFO("AI_RET payload: %.*s", (int)msg->length, (const char *)msg->payload);
        }
        
        if (g_ai_ret_callback != NULL) {
            g_ai_ret_callback(msg->payload, msg->length, g_ai_ret_userdata);
        } else {
            PR_INFO("No AI_RET callback registered, message ignored");
        }
    }
}

/**
 * @brief Additional MQTT client subscribed callback
 */
static void additional_mqtt_client_subscribed_cb(void *client, uint16_t msgid, void *userdata)
{
    PR_DEBUG("Additional MQTT subscribe successed ID:%d", msgid);
    
    /* Check if this is AI_RET topic subscription success */
    if (msgid == g_ai_ret_subscribe_msgid && g_ai_ret_callback != NULL) {
        PR_INFO("AI_RET topic subscribed successfully");
    }
}

/**
 * @brief Additional MQTT client published callback
 */
static void additional_mqtt_client_puback_cb(void *client, uint16_t msgid, void *userdata)
{
    PR_DEBUG("Additional MQTT PUBACK successed ID:%d", msgid);
}

/**
 * @brief Send data to AI_CMD topic
 *
 * @param data Data to send
 * @param length Data length
 * @param qos Quality of Service level (0, 1, or 2)
 * @return uint16_t Message ID if successful, 0 if failed
 */
uint16_t ai_cmd_send(const uint8_t *data, size_t length, uint8_t qos)
{
    if (g_additional_mqtt_client_ctx == NULL) {
        PR_ERR("Additional MQTT client not initialized");
        return 0;
    }
    
    if (!g_additional_mqtt_connected) {
        PR_ERR("Additional MQTT client not connected");
        return 0;
    }
    
    if (data == NULL || length == 0) {
        PR_ERR("Invalid parameters: data is NULL or length is 0");
        return 0;
    }
    
    uint16_t msgid = mqtt_client_publish(g_additional_mqtt_client_ctx, AI_CMD_TOPIC, data, length, qos);
    if (msgid > 0) {
        PR_DEBUG("Send AI_CMD message success, ID:%d, length:%zu", msgid, length);
    } else {
        PR_ERR("Send AI_CMD message failed");
    }
    
    return msgid;
}

/**
 * @brief Register callback for AI_RET topic messages
 *
 * @param callback Callback function to handle AI_RET messages
 * @param userdata User data to pass to callback
 * @return int 0 on success, -1 on failure
 */
int ai_ret_register_callback(ai_ret_message_cb_t callback, void *userdata)
{
    if (callback == NULL) {
        PR_ERR("Callback function is NULL");
        return -1;
    }
    
    g_ai_ret_callback = callback;
    g_ai_ret_userdata = userdata;
    PR_DEBUG("AI_RET callback registered");
    
    return 0;
}

/**
 * @brief Unregister AI_RET callback
 */
void ai_ret_unregister_callback(void)
{
    g_ai_ret_callback = NULL;
    g_ai_ret_userdata = NULL;
    PR_DEBUG("AI_RET callback unregistered");
}

/**
 * @brief Initialize and start additional MQTT client
 */
static void additional_mqtt_client_init(void)
{
    PR_DEBUG("Start additional MQTT client to broker");

    /* Allocate MQTT client context */
    g_additional_mqtt_client_ctx = mqtt_client_new();
    if (g_additional_mqtt_client_ctx == NULL) {
        PR_ERR("Additional MQTT client allocation failed");
        return;
    }

    mqtt_client_status_t mqtt_status;
    const mqtt_client_config_t mqtt_config = {
        .cacert = NULL,
        .cacert_len = 0,
        .host = "192.168.100.132",
        .port = 1883,
        .keepalive = MQTT_KEEPALIVE_INTERVALIN,
        .timeout_ms = MATOP_TIMEOUT_MS_DEFAULT,
        .clientid = "tuya-open-sdk-for-device-01",
        .username = "emqx",
        .password = "public",
        .on_connected = additional_mqtt_client_connected_cb,
        .on_disconnected = additional_mqtt_client_disconnected_cb,
        .on_message = additional_mqtt_client_message_cb,
        .on_subscribed = additional_mqtt_client_subscribed_cb,
        .on_published = additional_mqtt_client_puback_cb,
        .userdata = NULL
    };
    
    mqtt_status = mqtt_client_init(g_additional_mqtt_client_ctx, &mqtt_config);
    if (mqtt_status != MQTT_STATUS_SUCCESS) {
        PR_ERR("Additional MQTT init failed: Status = %d.", mqtt_status);
        mqtt_client_free(g_additional_mqtt_client_ctx);
        g_additional_mqtt_client_ctx = NULL;
        return;
    }

    mqtt_status = mqtt_client_connect(g_additional_mqtt_client_ctx);
    if (MQTT_STATUS_NOT_AUTHORIZED == mqtt_status) {
        PR_ERR("Additional MQTT connect fail:%d", mqtt_status);
        mqtt_client_deinit(g_additional_mqtt_client_ctx);
        mqtt_client_free(g_additional_mqtt_client_ctx);
        g_additional_mqtt_client_ctx = NULL;
        return;
    }
    
    PR_INFO("Additional MQTT client initialization started");
}

/**
 * @brief user defined upgrade notify callback, it will notify device a OTA request received
 *
 * @param client device info
 * @param upgrade the upgrade request info
 * @return void
 */
void user_upgrade_notify_on(tuya_iot_client_t *client, cJSON *upgrade)
{
    PR_INFO("----- Upgrade information -----");
    PR_INFO("OTA Channel: %d", cJSON_GetObjectItem(upgrade, "type")->valueint);
    PR_INFO("Version: %s", cJSON_GetObjectItem(upgrade, "version")->valuestring);
    PR_INFO("Size: %s", cJSON_GetObjectItem(upgrade, "size")->valuestring);
    PR_INFO("MD5: %s", cJSON_GetObjectItem(upgrade, "md5")->valuestring);
    PR_INFO("HMAC: %s", cJSON_GetObjectItem(upgrade, "hmac")->valuestring);
    PR_INFO("URL: %s", cJSON_GetObjectItem(upgrade, "url")->valuestring);
    PR_INFO("HTTPS URL: %s", cJSON_GetObjectItem(upgrade, "httpsUrl")->valuestring);
}

OPERATE_RET audio_dp_obj_proc(dp_obj_recv_t *dpobj)
{
    uint32_t index = 0;
    for (index = 0; index < dpobj->dpscnt; index++) {
        dp_obj_t *dp = dpobj->dps + index;
        PR_DEBUG("idx:%d dpid:%d type:%d ts:%u", index, dp->id, dp->type, dp->time_stamp);

        switch (dp->id) {
        case DPID_VOLUME: {
            uint8_t volume = dp->value.dp_value;
            PR_DEBUG("volume:%d", volume);
            if(volume > 90) {
                //发送数据
                const char *test_cmd = "新建一个文件夹，名字随机}";
                uint16_t cmd_msgid = ai_cmd_send((const uint8_t *)test_cmd, strlen(test_cmd), MQTT_QOS_1);
                if (cmd_msgid > 0) {
                    PR_INFO("Test command sent to AI_CMD, msgid: %d", cmd_msgid);
                } else {
                    PR_ERR("Failed to send test command to AI_CMD");
                }
            }
            ai_audio_set_volume(volume);
#if defined(ENABLE_CHAT_DISPLAY) && (ENABLE_CHAT_DISPLAY == 1)
            char volume_str[20] = {0};
            snprintf(volume_str, sizeof(volume_str), "%s%d", VOLUME, volume);
            app_display_send_msg(TY_DISPLAY_TP_NOTIFICATION, (uint8_t *)volume_str, strlen(volume_str));
#endif
            break;
        }
        default:
            break;
        }
    }

    return OPRT_OK;
}

OPERATE_RET ai_audio_volume_upload(void)
{
    tuya_iot_client_t *client = tuya_iot_client_get();
    dp_obj_t dp_obj = {0};

    uint8_t volume = ai_audio_get_volume();

    dp_obj.id = DPID_VOLUME;
    dp_obj.type = PROP_VALUE;
    dp_obj.value.dp_value = volume;

    PR_DEBUG("DP upload volume:%d", volume);

    return tuya_iot_dp_obj_report(client, client->activate.devid, &dp_obj, 1, 0);
}

/**
 * @brief user defined event handler
 *
 * @param client device info
 * @param event the event info
 * @return void
 */
void user_event_handler_on(tuya_iot_client_t *client, tuya_event_msg_t *event)
{
    PR_DEBUG("Tuya Event ID:%d(%s)", event->id, EVENT_ID2STR(event->id));
    PR_INFO("Device Free heap %d", tal_system_get_free_heap_size());

    switch (event->id) {
    case TUYA_EVENT_BIND_START:
        PR_INFO("Device Bind Start!");
        if (_need_reset == 1) {
            PR_INFO("Device Reset!");
            tal_system_reset();
        }

        ai_audio_player_play_alert(AI_AUDIO_ALERT_NETWORK_CFG);
        break;

    /* Print the QRCode for Tuya APP bind */
    case TUYA_EVENT_DIRECT_MQTT_CONNECTED: {
#if defined(ENABLE_QRCODE) && (ENABLE_QRCODE == 1)
        char buffer[255];
        sprintf(buffer, "https://smartapp.tuya.com/s/p?p=%s&uuid=%s&v=2.0", TUYA_PRODUCT_ID, license.uuid);
        qrcode_string_output(buffer, user_log_output_cb, 0);
#endif
    } break;

    case TUYA_EVENT_BIND_TOKEN_ON:
        break;

    /* MQTT with tuya cloud is connected, device online */
    case TUYA_EVENT_MQTT_CONNECTED:
        PR_INFO("Device MQTT Connected!");
        tal_event_publish(EVENT_MQTT_CONNECTED, NULL);

        static uint8_t first = 1;
        if (first) {
            first = 0;

#if defined(ENABLE_CHAT_DISPLAY) && (ENABLE_CHAT_DISPLAY == 1)
            UI_WIFI_STATUS_E wifi_status = UI_WIFI_STATUS_GOOD;
            app_display_send_msg(TY_DISPLAY_TP_NETWORK, (uint8_t *)&wifi_status, sizeof(UI_WIFI_STATUS_E));
#endif

            ai_audio_player_play_alert(AI_AUDIO_ALERT_NETWORK_CONNECTED);
            ai_audio_volume_upload();
            
            /* Initialize additional MQTT client after Tuya MQTT is connected */
            additional_mqtt_client_init();
        }
        break;

    /* MQTT with tuya cloud is disconnected, device offline */
    case TUYA_EVENT_MQTT_DISCONNECT:
        PR_INFO("Device MQTT DisConnected!");
        tal_event_publish(EVENT_MQTT_DISCONNECTED, NULL);
        break;

    /* RECV upgrade request */
    case TUYA_EVENT_UPGRADE_NOTIFY:
        user_upgrade_notify_on(client, event->value.asJSON);
        break;

    /* Sync time with tuya Cloud */
    case TUYA_EVENT_TIMESTAMP_SYNC:
        PR_INFO("Sync timestamp:%d", event->value.asInteger);
        tal_time_set_posix(event->value.asInteger, 1);
        break;

    case TUYA_EVENT_RESET:
        PR_INFO("Device Reset:%d", event->value.asInteger);

        _need_reset = 1;
        break;

    /* RECV OBJ DP */
    case TUYA_EVENT_DP_RECEIVE_OBJ: {
        dp_obj_recv_t *dpobj = event->value.dpobj;
        PR_DEBUG("SOC Rev DP Cmd t1:%d t2:%d CNT:%u", dpobj->cmd_tp, dpobj->dtt_tp, dpobj->dpscnt);
        if (dpobj->devid != NULL) {
            PR_DEBUG("devid.%s", dpobj->devid);
        }

        audio_dp_obj_proc(dpobj);

        tuya_iot_dp_obj_report(client, dpobj->devid, dpobj->dps, dpobj->dpscnt, 0);

    } break;

    /* RECV RAW DP */
    case TUYA_EVENT_DP_RECEIVE_RAW: {
        dp_raw_recv_t *dpraw = event->value.dpraw;
        PR_DEBUG("SOC Rev DP Cmd t1:%d t2:%d", dpraw->cmd_tp, dpraw->dtt_tp);
        if (dpraw->devid != NULL) {
            PR_DEBUG("devid.%s", dpraw->devid);
        }

        uint32_t index = 0;
        dp_raw_t *dp = &dpraw->dp;
        PR_DEBUG("dpid:%d type:RAW len:%d data:", dp->id, dp->len);
        for (index = 0; index < dp->len; index++) {
            PR_DEBUG_RAW("%02x", dp->data[index]);
        }

        tuya_iot_dp_raw_report(client, dpraw->devid, &dpraw->dp, 3);

    } break;

    default:
        break;
    }
}

/**
 * @brief user defined network check callback, it will check the network every 1sec,
 *        in this demo it alwasy return ture due to it's a wired demo
 *
 * @return true
 * @return false
 */
bool user_network_check(void)
{
    netmgr_status_e status = NETMGR_LINK_DOWN;
    netmgr_conn_get(NETCONN_AUTO, NETCONN_CMD_STATUS, &status);
    return status == NETMGR_LINK_DOWN ? false : true;
}

void user_main(void)
{
    int ret = OPRT_OK;

    //! open iot development kit runtim init
    cJSON_InitHooks(&(cJSON_Hooks){.malloc_fn = tal_malloc, .free_fn = tal_free});
    tal_log_init(TAL_LOG_LEVEL_DEBUG, 1024, (TAL_LOG_OUTPUT_CB)tkl_log_output);

    PR_NOTICE("Application information:");
    PR_NOTICE("Project name:        %s", PROJECT_NAME);
    PR_NOTICE("App version:         %s", PROJECT_VERSION);
    PR_NOTICE("Compile time:        %s", __DATE__);
    PR_NOTICE("TuyaOpen version:    %s", OPEN_VERSION);
    PR_NOTICE("TuyaOpen commit-id:  %s", OPEN_COMMIT);
    PR_NOTICE("Platform chip:       %s", PLATFORM_CHIP);
    PR_NOTICE("Platform board:      %s", PLATFORM_BOARD);
    PR_NOTICE("Platform commit-id:  %s", PLATFORM_COMMIT);

    tal_kv_init(&(tal_kv_cfg_t){
        .seed = "vmlkasdh93dlvlcy",
        .key = "dflfuap134ddlduq",
    });
    tal_sw_timer_init();
    tal_workq_init();
    tal_time_service_init();
    tal_cli_init();
    tuya_authorize_init();

    reset_netconfig_start();

    if (OPRT_OK != tuya_authorize_read(&license)) {
        license.uuid = TUYA_OPENSDK_UUID;
        license.authkey = TUYA_OPENSDK_AUTHKEY;
        PR_WARN("Replace the TUYA_OPENSDK_UUID and TUYA_OPENSDK_AUTHKEY contents, otherwise the demo cannot work.\n \
                Visit https://platform.tuya.com/purchase/index?type=6 to get the open-sdk uuid and authkey.");
    }

    /* Initialize Tuya device configuration */
    ret = tuya_iot_init(&ai_client, &(const tuya_iot_config_t){
                                        .software_ver = PROJECT_VERSION,
                                        .productkey = TUYA_PRODUCT_ID,
                                        .uuid = license.uuid,
                                        .authkey = license.authkey,
                                        // .firmware_key      = TUYA_DEVICE_FIRMWAREKEY,
                                        .event_handler = user_event_handler_on,
                                        .network_check = user_network_check,
                                    });
    assert(ret == OPRT_OK);

#if defined(ENABLE_LIBLWIP) && (ENABLE_LIBLWIP == 1)
    TUYA_LwIP_Init();
#endif

    // network init
    netmgr_type_e type = 0;
#if defined(ENABLE_WIFI) && (ENABLE_WIFI == 1)
    type |= NETCONN_WIFI;
#endif
#if defined(ENABLE_WIRED) && (ENABLE_WIRED == 1)
    type |= NETCONN_WIRED;
#endif
    netmgr_init(type);
#if defined(ENABLE_WIFI) && (ENABLE_WIFI == 1)
    netmgr_conn_set(NETCONN_WIFI, NETCONN_CMD_NETCFG, &(netcfg_args_t){.type = NETCFG_TUYA_BLE});
#endif

    PR_DEBUG("tuya_iot_init success");

    ret = board_register_hardware();
    if (ret != OPRT_OK) {
        PR_ERR("board_register_hardware failed");
    }

    ret = app_chat_bot_init();
    if (ret != OPRT_OK) {
        PR_ERR("tuya_audio_recorde_init failed");
    }

    app_system_info();

    /* Start tuya iot task */
    tuya_iot_start(&ai_client);

    tkl_wifi_set_lp_mode(0, 0);

    reset_netconfig_check();

    for (;;) {
        /* Loop to receive packets, and handles client keepalive */
        tuya_iot_yield(&ai_client);
        
        /* Process additional MQTT client messages */
        if (g_additional_mqtt_client_ctx != NULL) {
            mqtt_client_yield(g_additional_mqtt_client_ctx);
        }
    }
}

/**
 * @brief main
 *
 * @param argc
 * @param argv
 * @return void
 */
#if OPERATING_SYSTEM == SYSTEM_LINUX
void main(int argc, char *argv[])
{
    user_main();
}
#else

/* Tuya thread handle */
static THREAD_HANDLE ty_app_thread = NULL;

/**
 * @brief  task thread
 *
 * @param[in] arg:Parameters when creating a task
 * @return none
 */
static void tuya_app_thread(void *arg)
{
    user_main();

    tal_thread_delete(ty_app_thread);
    ty_app_thread = NULL;
}

void tuya_app_main(void)
{
    THREAD_CFG_T thrd_param = {4096, 4, "tuya_app_main"};
    tal_thread_create_and_start(&ty_app_thread, NULL, NULL, tuya_app_thread, NULL, &thrd_param);
}
#endif
