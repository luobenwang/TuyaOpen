/**
 * @file examples_mqtt_client.c
 * @brief Demonstrates mqtt client usage in Tuya SDK applications.
 *
 * This file provides an example of how to use the mqtt client interface provided by the Tuya SDK to connect to mqtt
 * broker. It includes initializing the SDK, setting up network connections (both WiFi and wired, depending on the
 * configuration), initializing the mqtt client and connect to the mqtt broker, and subscribe/unsubscribe topics,
 * publish/receive message. The example also demonstrates how to publ network link status changes and perform cleanups.
 *
 * Key operations demonstrated in this file:
 * - Initialization of the Tuya SDK and network manager.
 * - initializing mqtt client and connect to mqtt broker.
 * - Handling connect ack and subscribe topics
 * - handling subscribe ack and publish messages
 * - handling publish ack and disconnect the mqtt broker
 * - Cleanup and resource management.
 *
 * AI MQTT Interface Usage:
 * - To send data to AI_CMD topic:
 *   uint16_t msgid = ai_cmd_send(data, length, MQTT_QOS_1);
 *
 * - To receive messages from AI_RET topic:
 *   // Define callback function
 *   void my_ai_ret_callback(const uint8_t *payload, size_t length, void *userdata) {
 *       // Process received message
 *   }
 *   // Register callback
 *   ai_ret_register_callback(my_ai_ret_callback, userdata);
 *
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 *
 */

#include <string.h>
#include "tuya_cloud_types.h"
#include "mqtt_client_interface.h"
#include "tuya_config_defaults.h"
#include "core_mqtt_config.h"
#include "core_mqtt.h"
#include "tuya_transporter.h"
#include "backoff_algorithm.h"
#include "tal_api.h"
#include "tkl_output.h"
#include "netmgr.h"
#define ENABLE_WIFI 1
#if defined(ENABLE_WIFI) && (ENABLE_WIFI == 1)
#include "netconn_wifi.h"
#endif
#if defined(ENABLE_WIRED) && (ENABLE_WIRED == 1)
#include "netconn_wired.h"
#endif

/***********************************************************
*********************** macro define ***********************
***********************************************************/
#ifdef ENABLE_WIFI
#define DEFAULT_WIFI_SSID "2903"
#define DEFAULT_WIFI_PSWD "12345678"
#endif

/* AI MQTT topics */
#define AI_CMD_TOPIC "AI_CMD"
#define AI_RET_TOPIC "AI_RET"
/***********************************************************
********************** typedef define **********************
***********************************************************/
typedef struct {
    mqtt_client_config_t config;
    MQTTContext_t mqclient;
    tuya_transporter_t network;
    uint8_t mqttbuffer[CORE_MQTT_BUFFER_SIZE];
} mqtt_client_context_t;

/* AI message callback function type */
typedef void (*ai_ret_message_cb_t)(const uint8_t *payload, size_t length, void *userdata);

/***********************************************************
********************** variable define *********************
***********************************************************/
static netmgr_status_e netmgr_status = NETMGR_LINK_DOWN;
static mqtt_client_context_t *g_mqtt_client_ctx = NULL;
static ai_ret_message_cb_t g_ai_ret_callback = NULL;
static void *g_ai_ret_userdata = NULL;
static uint16_t g_ai_ret_subscribe_msgid = 0;  // Track AI_RET subscription msgid

/***********************************************************
********************** function define *********************
***********************************************************/
/* Forward declarations */
uint16_t ai_cmd_send(const uint8_t *data, size_t length, uint8_t qos);
int ai_ret_register_callback(ai_ret_message_cb_t callback, void *userdata);
void ai_ret_unregister_callback(void);

static void mqtt_client_connected_cb(void *client, void *userdata)
{
    PR_INFO("mqtt client connected! try to subscribe tuya/tos-test");
    uint16_t msgid = mqtt_client_subscribe(client, "tuya/tos-test", MQTT_QOS_0);
    if (msgid <= 0) {
        PR_ERR("Subscribe failed!");
    }
    PR_DEBUG("Subscribe topic tuya/tos-test ID:%d", msgid);
    
    /* Subscribe to AI_RET topic to receive AI responses */
    msgid = mqtt_client_subscribe(client, AI_RET_TOPIC, MQTT_QOS_1);
    if (msgid > 0) {
        g_ai_ret_subscribe_msgid = msgid;
        PR_INFO("Subscribe AI_RET topic success, ID:%d", msgid);
    } else {
        PR_ERR("Subscribe AI_RET topic failed!");
    }
}

static void mqtt_client_disconnected_cb(void *client, void *userdata)
{
    PR_INFO("mqtt client disconnected!");

    // PR_DEBUG("MQTT Client Deinit");
    // mqtt_client_deinit(client);
}

/**
 * @brief AI_RET message callback handler
 *
 * @param payload Message payload
 * @param length Message length
 * @param userdata User data
 */
static void ai_ret_message_handler(const uint8_t *payload, size_t length, void *userdata)
{
    PR_INFO("AI_RET message received, length: %zu", length);
    if (payload != NULL && length > 0) {
        PR_DEBUG("AI_RET payload: %.*s", (int)length, (const char *)payload);
        // TODO: Process AI_RET message here
    }
}

static void mqtt_client_message_cb(void *client, uint16_t msgid, const mqtt_client_message_t *msg, void *userdata)
{
    PR_DEBUG("recv message TopicName:%s, payload len:%d", msg->topic, msg->length);
    
    /* Handle AI_RET topic messages */
    if (msg->topic != NULL && strcmp(msg->topic, AI_RET_TOPIC) == 0) {
        PR_DEBUG("Received AI_RET message, length: %zu", msg->length);
        if (g_ai_ret_callback != NULL) {
            g_ai_ret_callback(msg->payload, msg->length, g_ai_ret_userdata);
        } else {
            PR_DEBUG("No AI_RET callback registered, message ignored");
        }
    }
}

static void mqtt_client_subscribed_cb(void *client, uint16_t msgid, void *userdata)
{
    PR_DEBUG("Subscribe successed ID:%d", msgid);
    
    /* Check if this is AI_RET topic subscription success */
    if (msgid == g_ai_ret_subscribe_msgid && g_ai_ret_callback != NULL) {
        PR_INFO("AI_RET topic subscribed successfully, sending test command to AI_CMD");
        //发送数据
        const char *test_cmd = "{\"cmd\":\"test\",\"data\":\"hello from device\"}";
        uint16_t cmd_msgid = ai_cmd_send((const uint8_t *)test_cmd, strlen(test_cmd), MQTT_QOS_1);
        if (cmd_msgid > 0) {
            PR_INFO("Test command sent to AI_CMD, msgid: %d", cmd_msgid);
        } else {
            PR_ERR("Failed to send test command to AI_CMD");
        }
    }
    
    uint16_t new_msgid = mqtt_client_publish(client, "tuya/tos-test", (const uint8_t *)"hello, tuya-open-sdk-for-device",
                                             strlen("hello, tuya-open-sdk-for-device") + 1, MQTT_QOS_1);
    if (new_msgid <= 0) {
        PR_ERR("Publish failed!");
    }
    PR_DEBUG("Publish msg ID:%d", new_msgid);
}

static void mqtt_client_puback_cb(void *client, uint16_t msgid, void *userdata)
{
    PR_DEBUG("PUBACK successed ID:%d", msgid);
    PR_DEBUG("UnSubscribe topic tuya/tos-test");
    mqtt_client_unsubscribe(client, "tuya/tos-test", MQTT_QOS_0);

    // PR_DEBUG("MQTT Client Disconnect");
    // mqtt_client_disconnect(client);
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
    if (g_mqtt_client_ctx == NULL) {
        PR_ERR("MQTT client not initialized");
        return 0;
    }
    
    if (data == NULL || length == 0) {
        PR_ERR("Invalid parameters: data is NULL or length is 0");
        return 0;
    }
    
    uint16_t msgid = mqtt_client_publish(g_mqtt_client_ctx, AI_CMD_TOPIC, data, length, qos);
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

static void mqtt_client_example(void)
{
    PR_DEBUG("start mqtt client to broker.emqx.io");

    /* Register AI_RET callback before connecting */
    ai_ret_register_callback(ai_ret_message_handler, NULL);
    PR_INFO("AI_RET callback registered");

    /* MQTT Client init */
    static mqtt_client_context_t mqtt_client = {0};
    g_mqtt_client_ctx = &mqtt_client;
    mqtt_client_status_t mqtt_status;
    const mqtt_client_config_t mqtt_config = {.cacert = NULL,
                                              .cacert_len = 0,
                                              .host = "192.168.100.132",
                                              .port = 1883,
                                              .keepalive = MQTT_KEEPALIVE_INTERVALIN,
                                              .timeout_ms = MATOP_TIMEOUT_MS_DEFAULT,
                                              .clientid = "tuya-open-sdk-for-device-01",
                                              .username = "emqx",
                                              .password = "public",
                                              .on_connected = mqtt_client_connected_cb,
                                              .on_disconnected = mqtt_client_disconnected_cb,
                                              .on_message = mqtt_client_message_cb,
                                              .on_subscribed = mqtt_client_subscribed_cb,
                                              .on_published = mqtt_client_puback_cb,
                                              .userdata = NULL};
    mqtt_status = mqtt_client_init(&mqtt_client, &mqtt_config);
    if (mqtt_status != MQTT_STATUS_SUCCESS) {
        PR_ERR("MQTT init failed: Status = %d.", mqtt_status);
        return;
    }

    mqtt_status = mqtt_client_connect(&mqtt_client);
    if (MQTT_STATUS_NOT_AUTHORIZED == mqtt_status) {
        PR_ERR("MQTT connect fail:%d", mqtt_status);
        return;
    }

    /* Keep calling mqtt_client_yield to process MQTT messages */
    while (1) {
        mqtt_client_yield(&mqtt_client);
        tal_system_sleep(100);
    }
}

/**
 * @brief  __link_status_cb
 *
 * @param[in] param:Task parameters
 * @return none
 */
OPERATE_RET __link_status_cb(void *data)
{
    PR_DEBUG("link status changed: %d", (netmgr_status_e)data);
    if (netmgr_status == (netmgr_status_e)data && NETMGR_LINK_UP == (netmgr_status_e)data)
        return OPRT_OK;

    netmgr_status = (netmgr_status_e)data;

    return OPRT_OK;
}

/**
 * @brief user_main
 *
 * @return void
 */
void user_main(void)
{
    //OPERATE_RET rt = OPRT_OK;

    /* basic init */
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
    tal_event_subscribe(EVENT_LINK_STATUS_CHG, "mqtt_client", __link_status_cb, SUBSCRIBE_TYPE_NORMAL);

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
    netconn_wifi_info_t wifi_info = {0};
    // connect wifi
    strcpy(wifi_info.ssid, DEFAULT_WIFI_SSID);
    strcpy(wifi_info.pswd, DEFAULT_WIFI_PSWD);
    netmgr_conn_set(NETCONN_WIFI, NETCONN_CMD_SSID_PSWD, &wifi_info);
#endif

    while (1) {
        if (netmgr_status == NETMGR_LINK_UP) {
            mqtt_client_example();
            /* mqtt_client_example() will run in a loop, so this break is not reached */
            break;
        } else {
            tal_system_sleep(50);
        }
    }

    /* Keep the thread alive */
    while (1) {
        tal_system_sleep(1000);
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
    while (1) {
        tal_system_sleep(500);
    }
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
