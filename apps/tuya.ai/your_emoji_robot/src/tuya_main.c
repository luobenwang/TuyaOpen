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

#include "app_chat_bot.h"
#include "ai_audio.h"
#include "reset_netcfg.h"
#include "app_system_info.h"

/* Tuya device handle */
tuya_iot_client_t ai_client;

#ifndef PROJECT_VERSION
#define PROJECT_VERSION "1.0.0"
#endif

#define DPID_VOLUME 3



static uint8_t _need_reset = 0;

#if 1 // moji robot
#define SERVO_CHANNEL TUYA_PWM_NUM_0  // 假设舵机连接在通道0，根据实际情况修改
#define SERVO_FREQUENCY 50  // 舵机PWM频率，单位Hz，通常为50Hz
#define SERVO_POLARITY TUYA_PWM_POSITIVE  // 舵机极性，根据实际情况设置
#define MIN_DUTY 500    // 最小占空比，对应舵机的一个极限位置（例如0度）
#define MAX_DUTY 2500  // 最大占空比，对应舵机的一个极限位置（例如180度）
#define MID_DUTY 1500  // 中间占空比，对应舵机的中间位置（例如90度）
#define DUTY_STEP 100

static int target_duty1 = 0;
static TIMER_ID robot_timer_id1;
static BOOL_T is_postive1 = FALSE;

// 定时器回调函数
void robot_timer1_callback(TIMER_ID robot_timer_id1, void *arg)
{
    static int step = MIN_DUTY;
    int cur_duty = MID_DUTY;
    // 打印回调信息
    PR_DEBUG("Timer callback triggered! Timer ID: %p, Argument: %p\n", robot_timer_id1, arg);
    PR_DEBUG("tkl_pwm_duty_set 0");

    TUYA_PWM_BASE_CFG_T info = {0};
    tkl_pwm_info_get(TUYA_PWM_NUM_0, &info);
    cur_duty = info.duty;

    if (cur_duty > target_duty1) {
        is_postive1 = FALSE;
    } else {
        is_postive1 = TRUE;
    }
    
    // tkl_pwm_duty_set(TUYA_PWM_NUM_1, 0);
    // 在这里可以添加需要执行的代码
    if (is_postive1) { // 正转
        if (cur_duty < target_duty1) {
            cur_duty += DUTY_STEP;
            tal_sw_timer_start(robot_timer_id1, 200, TAL_TIMER_ONCE);
        } else {
            cur_duty = target_duty1;
        }
        tkl_pwm_duty_set(TUYA_PWM_NUM_0, cur_duty);
    } else { // 反转
        if (cur_duty > target_duty1) {
            cur_duty -= DUTY_STEP;
            tal_sw_timer_start(robot_timer_id1, 200, TAL_TIMER_ONCE);
        } else {
            cur_duty = target_duty1;
        }
        tkl_pwm_duty_set(TUYA_PWM_NUM_0, cur_duty);
    }

    PR_DEBUG("cur_duty=%d, target_duty1=%d", cur_duty, target_duty1);
}

static int target_duty2 = 0;
static TIMER_ID robot_timer_id2;
static BOOL_T is_postive2 = FALSE;

// 电机2 定时器回调函数
void robot_timer2_callback(TIMER_ID robot_timer_id2, void *arg)
{
    static int step = MIN_DUTY;
    int cur_duty = MID_DUTY;
    // 打印回调信息
    PR_DEBUG("Timer callback triggered! Timer ID: %p, Argument: %p\n", robot_timer_id2, arg);

    TUYA_PWM_BASE_CFG_T info = {0};
    tkl_pwm_info_get(TUYA_PWM_NUM_1, &info);
    cur_duty = info.duty;
    PR_DEBUG("cur_duty=%d, target_duty2=%d", cur_duty, target_duty2);
    if (cur_duty > target_duty2) {
        is_postive2 = FALSE;
    } else {
        is_postive2 = TRUE;
    }
    
    // tkl_pwm_duty_set(TUYA_PWM_NUM_1, 0);
    // 在这里可以添加需要执行的代码
    if (is_postive2) { // 正转
        if (cur_duty < target_duty2) {
            cur_duty += DUTY_STEP;
            tal_sw_timer_start(robot_timer_id2, 200, TAL_TIMER_ONCE);
        } else {
            cur_duty = target_duty2;
        }
        tkl_pwm_duty_set(TUYA_PWM_NUM_1, cur_duty);
    } else { // 反转
        if (cur_duty > target_duty2) {
            cur_duty -= DUTY_STEP;
            tal_sw_timer_start(robot_timer_id2, 200, TAL_TIMER_ONCE);
        } else {
            cur_duty = target_duty2;
        }
        tkl_pwm_duty_set(TUYA_PWM_NUM_1, cur_duty);
    }

    tkl_pwm_info_get(TUYA_PWM_NUM_1, &info);
    cur_duty = info.duty;
    PR_DEBUG("set after,cur_duty=%d, target_duty2=%d", cur_duty, target_duty2);
}



static OPERATE_RET _report_status(int now_sta)
{
    // report volume dp to cloud
    PR_DEBUG("report volume dp to cloud");
    tuya_iot_client_t *client = tuya_iot_client_get();

    dp_obj_t dp_obj = {
        .id = 2,
        .type = PROP_ENUM,
        .value.dp_enum = now_sta,
        
    };
    PR_DEBUG("DP upload now_sta:%d", now_sta);

    return tuya_iot_dp_obj_report(client, client->activate.devid, &dp_obj, 1, 0);
}
/**
 * @brief SOC device format command data delivery entry
 *
 * @param[in] dp: obj dp info
 *
 * @return none
 */
void emoji_move_form_dp_control(dp_obj_recv_t *dp)
{
    OPERATE_RET rt = OPRT_OK;
    
    PR_DEBUG("SOC Rev DP Obj Cmd t1:%d t2:%d CNT:%u", dp->cmd_tp, dp->dtt_tp, dp->dpscnt);

    // invoke ai toy dp command callback
    // ty_ai_toy_dp_cmd_cb(dp);

    for (int index = 0; index < dp->dpscnt; index++) {
        // handle dp id = 5
        if (dp->dps[index].id == 5 && dp->dps[index].type == PROP_ENUM) {
            PR_DEBUG("SOC Rev DP Obj Cmd id:%d type:%d value:%d", dp->dps[index].id, dp->dps[index].type, dp->dps[index].value.dp_enum);
            if (dp->dps[index].value.dp_enum == 0 ) //前进 向上 电机1 正转
            {
                PR_DEBUG("************************************* value:%d", dp->dps[index].value.dp_enum);
                // tkl_pwm_duty_set(TUYA_PWM_NUM_0, MAX_DUTY);
                target_duty1 = MIN_DUTY;
                _report_status(3);
            }
            
            if (dp->dps[index].value.dp_enum == 1 ) //后退 向下 电机1 反转
            {
                PR_DEBUG("************************************* value:%d", dp->dps[index].value.dp_enum);
                // tkl_pwm_duty_set(TUYA_PWM_NUM_0, MIN_DUTY);
                target_duty1 = MAX_DUTY;
                _report_status(3);
            }

            if (dp->dps[index].value.dp_enum == 2 ) //前进 向上 电机1 正转
            {
                PR_DEBUG("************************************* value:%d", dp->dps[index].value.dp_enum);
                // tkl_pwm_duty_set(TUYA_PWM_NUM_0, MAX_DUTY);
                target_duty2 = MIN_DUTY;
                _report_status(3);
            }

            if (dp->dps[index].value.dp_enum == 3 ) //后退 向下 电机1 反转
            {
                PR_DEBUG("************************************* value:%d", dp->dps[index].value.dp_enum);
                // tkl_pwm_duty_set(TUYA_PWM_NUM_0, MIN_DUTY);
                target_duty2 = MAX_DUTY;
                _report_status(3);
                
            }

            if (dp->dps[index].value.dp_enum == 4 ) //左 电机2 回到中间位置
            {
                PR_DEBUG("************************************* value:%d", dp->dps[index].value.dp_enum);
                // tkl_pwm_duty_set(TUYA_PWM_NUM_0, MID_DUTY);
                target_duty1 = MID_DUTY;
                target_duty2 = MID_DUTY;
                _report_status(0);
            }


        
        }
  
        
        if (!tal_sw_timer_is_running(robot_timer_id1) && dp->dps[index].id == 5 &&(dp->dps[index].value.dp_enum == 0 ||  dp->dps[index].value.dp_enum == 1||  dp->dps[index].value.dp_enum == 4)) {
            tal_sw_timer_start(robot_timer_id1, 200, TAL_TIMER_ONCE);
            PR_DEBUG("**************************************tal_sw_timer_start robot_timer_id1");
        }

        if (!tal_sw_timer_is_running(robot_timer_id2) && dp->dps[index].id == 5 && (dp->dps[index].value.dp_enum == 2 ||  dp->dps[index].value.dp_enum == 3||  dp->dps[index].value.dp_enum == 4)) {
             PR_DEBUG("**************************************tal_sw_timer_start robot_timer_id2");
            tal_sw_timer_start(robot_timer_id2, 200, TAL_TIMER_ONCE);
        }

        
    }
 
}


OPERATE_RET servo_pwms_init(void)
{
    OPERATE_RET ret = OPRT_OK;
    TUYA_PWM_BASE_CFG_T pwm_cfg = {0};
    
    ret = tal_sw_timer_create(robot_timer1_callback, NULL, &robot_timer_id1);
    if (ret != OPRT_OK) {
        PR_DEBUG("Failed to robot_timer1_callback: %d\n", ret);
        return -1;
    }

    ret = tal_sw_timer_create(robot_timer2_callback, NULL, &robot_timer_id2);
    if (ret != OPRT_OK) {
        PR_DEBUG("Failed torobot_timer2_callback: %d\n", ret);
        return -1;
    }

    // 配置PWM参数(50Hz频率，对应20ms周期)
    pwm_cfg.polarity = TUYA_PWM_NEGATIVE;  // 正常极性
    pwm_cfg.count_mode = TUYA_PWM_CNT_UP;  // 中央对齐模式
    pwm_cfg.frequency = SERVO_FREQUENCY;  // 1KHz
    pwm_cfg.cycle = 20000;  // 20ms周期 = 20000us
    pwm_cfg.duty = MID_DUTY;     //MID_DUTY;    // 角度中
    
    // 初始化舵机1(PIN18)
    tkl_pwm_init(TUYA_PWM_NUM_0, &pwm_cfg);
    tkl_pwm_start(TUYA_PWM_NUM_0);

    // 初始化舵机2(PIN24)
    tkl_pwm_init(TUYA_PWM_NUM_1, &pwm_cfg);
    tkl_pwm_start(TUYA_PWM_NUM_1);

    return OPRT_OK;
}
#endif

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
            ai_audio_set_volume(volume);
            char volume_str[20] = {0};
#if defined(ENABLE_CHAT_DISPLAY) && (ENABLE_CHAT_DISPLAY == 1)
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

OPERATE_RET ai_audio_status_upload(void)
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
// 软重启，未配网，播报配网提示
#if defined(ENABLE_CHAT_DISPLAY) && (ENABLE_CHAT_DISPLAY == 1)
        app_display_send_msg(TY_DISPLAY_TP_STATUS, (uint8_t *)ENTERING_WIFI_CONFIG_MODE,
                             strlen(ENTERING_WIFI_CONFIG_MODE));
#endif
        ai_audio_player_play_alert(AI_AUDIO_ALERT_NETWORK_CFG);
        break;

    case TUYA_EVENT_BIND_TOKEN_ON:
#if defined(ENABLE_CHAT_DISPLAY) && (ENABLE_CHAT_DISPLAY == 1)
        app_display_send_msg(TY_DISPLAY_TP_STATUS, (uint8_t *)CONNECT_SERVER, strlen(CONNECT_SERVER));
#endif
        break;

    /* MQTT with tuya cloud is connected, device online */
    case TUYA_EVENT_MQTT_CONNECTED:
        PR_INFO("Device MQTT Connected!");
        tal_event_publish(EVENT_MQTT_CONNECTED, NULL);

        static uint8_t first = 1;
        if (first) {
            first = 0;
#if defined(ENABLE_CHAT_DISPLAY) && (ENABLE_CHAT_DISPLAY == 1)
            app_display_send_msg(TY_DISPLAY_TP_STATUS, (uint8_t *)CONNECTED_TO, strlen(CONNECTED_TO));
            app_system_info_loop_start();
#endif
            ai_audio_player_play_alert(AI_AUDIO_ALERT_NETWORK_CONNECTED);
            ai_audio_status_upload();
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
        emoji_move_form_dp_control(dpobj);
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
    tal_kv_init(&(tal_kv_cfg_t){
        .seed = "vmlkasdh93dlvlcy",
        .key = "dflfuap134ddlduq",
    });
    tal_sw_timer_init();
    tal_workq_init();
    tal_cli_init();
    tuya_authorize_init();

    reset_netconfig_start();

    tuya_iot_license_t license;

    if (OPRT_OK != tuya_authorize_read(&license)) {
        license.uuid = TUYA_OPENSDK_UUID;
        license.authkey = TUYA_OPENSDK_AUTHKEY;
        PR_WARN("Replace the TUYA_OPENSDK_UUID and TUYA_OPENSDK_AUTHKEY contents, otherwise the demo cannot work.\n \
                Visit https://platform.tuya.com/purchase/index?type=6 to get the open-sdk uuid and authkey.");
    }

    /* Initialize Tuya device configuration */
    ret = tuya_iot_init(&ai_client, &(const tuya_iot_config_t){
                                        .software_ver = PROJECT_VERSION,
                                        .productkey = TUYA_PRODUCT_KEY,
                                        .uuid = license.uuid,
                                        .authkey = license.authkey,
                                        // .firmware_key      = TUYA_DEVICE_FIRMWAREKEY,
                                        .event_handler = user_event_handler_on,
                                        .network_check = user_network_check,
                                    });
    assert(ret == OPRT_OK);

    // 初始化LWIP
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
    
#if defined(ENABLE_CHAT_DISPLAY) && (ENABLE_CHAT_DISPLAY == 1)
    app_display_init();
#endif

    ret = app_chat_bot_init();
    if (ret != OPRT_OK) {
        PR_ERR("tuya_audio_recorde_init failed");
    }

    app_system_info();

    /* Start tuya iot task */
    tuya_iot_start(&ai_client);

    tkl_wifi_set_lp_mode(0,0);

    reset_netconfig_check();

    servo_pwms_init();

    for (;;) {
        /* Loop to receive packets, and handles client keepalive */
        tuya_iot_yield(&ai_client);
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
