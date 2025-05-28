/**
 * @file ai_audio_main.c
 * @brief Main implementation file for the audio module, which handles audio initialization,
 *        volume control, open/close operations, and work mode settings.
 *
 * This file contains the core functions for initializing and managing the audio module,
 * including setting up input handling, volume management, and different work modes.
 *
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 */

#include "tuya_cloud_types.h"

#include "tkl_queue.h"
#include "tkl_memory.h"
#include "tkl_thread.h"
#include "tkl_asr.h"

#include "tdl_audio_manage.h"

#include "tal_api.h"
#include "ai_audio.h"

/***********************************************************
************************macro define************************
***********************************************************/
#define AI_AUDIO_SPEAK_VOLUME_KEY "spk_volume"

#define AI_AUDIO_INPUT_EVT_CHANGE(last_evt, new_evt)                                                                   \
    do {                                                                                                               \
        if (last_evt != new_evt) {                                                                                     \
            PR_DEBUG("ai audio event changed: %d->%d", last_evt, new_evt);                                             \
        }                                                                                                              \
    } while (0)

/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef struct {
    uint32_t frame_len;
    uint8_t *frame;
} AI_AUDIO_FRAME_MSG_T;

/***********************************************************
********************function declaration********************
***********************************************************/

/***********************************************************
***********************variable define**********************
***********************************************************/
static AI_AUDIO_INFORM_CB sg_ai_audio_inform_cb = NULL;
static AI_AUDIO_WORK_MODE_E sg_ai_audio_work_mode = AI_AUDIO_MODE_MANUAL_SINGLE_TALK;
static bool sg_is_chating = false;
/***********************************************************
***********************function define**********************
***********************************************************/
static void __ai_audio_agent_event_cb(AI_EVENT_TYPE event, AI_EVENT_ID event_id)
{
    PR_DEBUG("__ai_audio_agent_event_cb event: %d", event);

    switch (event) {
    case AI_EVENT_START:
    break;

    case AI_EVENT_END:
        sg_is_chating = false;
    break;

    case AI_EVENT_CHAT_BREAK:
        PR_DEBUG("chat break");
        if (ai_audio_player_is_playing()) {
            ai_audio_player_stop();
        }
        sg_is_chating = false;
    break;

    case AI_EVENT_SERVER_VAD:
        PR_DEBUG("chat break");
        if (ai_audio_player_is_playing()) {
            ai_audio_player_stop();
        }
        sg_is_chating = false;

    break;
    }

    return;
}

static void __ai_audio_agent_msg_cb(AI_AGENT_MSG_T *msg)
{
    AI_AUDIO_EVENT_E event = AI_AUDIO_EVT_NONE;
    static char *event_id = NULL;

    switch (msg->type) {
    case AI_AGENT_MSG_TP_TEXT_ASR: {
        if (msg->data_len > 0) {
            ai_audio_cloud_stop_wait_asr();

            event = AI_AUDIO_EVT_HUMAN_ASR_TEXT;

            if(AI_AUDIO_WORK_ASR_WAKEUP_FREE_TALK == sg_ai_audio_work_mode) {
                ai_audio_input_restart_asr_awake_timer();
            }
        }else {
            ai_audio_cloud_asr_set_idle(true);
        }
    } break;
    case AI_AGENT_MSG_TP_AUDIO_START: {
        // Prepare to play mp3
        if (ai_audio_player_is_playing()) {
            PR_DEBUG("player is playing, stop it first");
            ai_audio_player_stop();
        }
        if(event_id) {
            tkl_system_free(event_id);
            event_id = NULL;
        }

        event_id = tkl_system_malloc(msg->data_len+1);
        if(event_id) {
            memcpy(event_id, msg->data, msg->data_len);
            event_id[msg->data_len] = '\0';
        }

        ai_audio_player_start(event_id);
    } break;
    case AI_AGENT_MSG_TP_AUDIO_DATA: {
        ai_audio_player_data_write(event_id, msg->data, msg->data_len, 0);

        if(AI_AUDIO_WORK_ASR_WAKEUP_FREE_TALK == sg_ai_audio_work_mode) {
            ai_audio_input_restart_asr_awake_timer();
        }
    } break;
    case AI_AGENT_MSG_TP_AUDIO_STOP: {
        ai_audio_player_data_write(event_id, msg->data, msg->data_len, 1);

        if(AI_AUDIO_WORK_ASR_WAKEUP_FREE_TALK == sg_ai_audio_work_mode) {
            ai_audio_input_restart_asr_awake_timer();
        }else if(AI_AUDIO_WORK_ASR_WAKEUP_SINGLE_TALK == sg_ai_audio_work_mode) {
            if (sg_ai_audio_inform_cb) {
                sg_ai_audio_inform_cb(AI_AUDIO_EVT_ASR_WAKEUP_END, NULL, 0, NULL);
            }
        }

        if(event_id) {
            tkl_system_free(event_id);
            event_id = NULL;
        }
    } break;
    case AI_AGENT_MSG_TP_TEXT_NLG_START: {
        event = AI_AUDIO_EVT_AI_REPLIES_TEXT_START;
    } break;
    case AI_AGENT_MSG_TP_TEXT_NLG_DATA: {
        event = AI_AUDIO_EVT_AI_REPLIES_TEXT_DATA;
    } break;
    case AI_AGENT_MSG_TP_TEXT_NLG_STOP: {
        event = AI_AUDIO_EVT_AI_REPLIES_TEXT_END;
    } break;
    case AI_AGENT_MSG_TP_EMOTION: {
        event = AI_AUDIO_EVT_AI_REPLIES_EMO;
    } break;
    default:
        break;
    }

    if (sg_ai_audio_inform_cb && (AI_AUDIO_EVT_NONE != event)) {
        sg_ai_audio_inform_cb(event, msg->data, msg->data_len, NULL);
    }
}

static void __ai_audio_input_inform_handle(AI_AUDIO_INPUT_EVENT_E event, void *arg)
{
    static AI_AUDIO_INPUT_EVENT_E last_evt = 0xFF;

    AI_AUDIO_INPUT_EVT_CHANGE(last_evt, event);

    last_evt = event;

    switch (event) {
    case AI_AUDIO_INPUT_EVT_NONE:
        // do nothing
        break;
    case AI_AUDIO_INPUT_EVT_GET_VALID_VOICE_START: {
        ai_audio_cloud_asr_start();
        sg_is_chating = true;

    } break;
    case AI_AUDIO_INPUT_EVT_GET_VALID_VOICE_STOP: {
        ai_audio_cloud_asr_stop();

        if (AI_AUDIO_WORK_ASR_WAKEUP_SINGLE_TALK == sg_ai_audio_work_mode) {
            ai_audio_input_stop_asr_awake();
        }
    }
    break;
    case AI_AUDIO_INPUT_EVT_ASR_WAKEUP_WORD: {
        ai_audio_player_stop();

        ai_audio_player_play_alert(AI_AUDIO_ALERT_WAKEUP);

        if (true == sg_is_chating) {
            ai_audio_cloud_asr_set_idle(true);
            sg_is_chating = false;
        }

        if (sg_ai_audio_inform_cb) {
            sg_ai_audio_inform_cb(AI_AUDIO_EVT_ASR_WAKEUP, NULL, 0, NULL);
        }
    }
    break;
    case AI_AUDIO_INPUT_EVT_ASR_WAKEUP_STOP: {

        if(AI_AUDIO_WORK_ASR_WAKEUP_FREE_TALK == sg_ai_audio_work_mode) {
            if (sg_ai_audio_inform_cb) {
                sg_ai_audio_inform_cb(AI_AUDIO_EVT_ASR_WAKEUP_END, NULL, 0, NULL);
            }
        }
    }
    break;
    }
}

static AI_AUDIO_INPUT_VALID_METHOD_E __get_input_get_valid_data_method(AI_AUDIO_WORK_MODE_E work_mode)
{
    AI_AUDIO_INPUT_VALID_METHOD_E method = 0;

    if (work_mode == AI_AUDIO_MODE_MANUAL_SINGLE_TALK) {
        method = AI_AUDIO_INPUT_VALID_METHOD_MANUAL;
    } else if (work_mode == AI_AUDIO_WORK_VAD_FREE_TALK) {
        method = AI_AUDIO_INPUT_VALID_METHOD_VAD;
    } else if(work_mode == AI_AUDIO_WORK_ASR_WAKEUP_SINGLE_TALK){
        method = AI_AUDIO_INPUT_VALID_METHOD_ASR;
    }else if(work_mode == AI_AUDIO_WORK_ASR_WAKEUP_FREE_TALK){
        method = AI_AUDIO_INPUT_VALID_METHOD_ASR;
    } else {
        method = AI_AUDIO_INPUT_VALID_METHOD_VAD;
    }

    return method;
}

/**
 * @brief Initializes the audio module with the provided configuration.
 * @param cfg Pointer to the configuration structure for the audio module.
 * @return OPERATE_RET - OPRT_OK if initialization is successful, otherwise an error code.
 */
OPERATE_RET ai_audio_init(AI_AUDIO_CONFIG_T *cfg)
{
    OPERATE_RET rt = OPRT_OK;
    AI_AUDIO_INPUT_CFG_T input_cfg;
    AI_AGENT_CBS_T agent_cbs;

    if (NULL == cfg) {
        return OPRT_INVALID_PARM;
    }

    input_cfg.get_valid_data_method = __get_input_get_valid_data_method(cfg->work_mode);
    sg_ai_audio_work_mode = cfg->work_mode;

    TUYA_CALL_ERR_RETURN(ai_audio_input_init(&input_cfg, __ai_audio_input_inform_handle));

    TDL_AUDIO_HANDLE_T audio_hdl = NULL;
    TUYA_CALL_ERR_RETURN(tdl_audio_find(AUDIO_DRIVER_NAME, &audio_hdl));
    TUYA_CALL_ERR_RETURN(tdl_audio_volume_set(audio_hdl, ai_audio_get_volume()));

    TUYA_CALL_ERR_RETURN(ai_audio_cloud_asr_init());

    TUYA_CALL_ERR_RETURN(ai_audio_player_init());

    agent_cbs.ai_agent_msg_cb   = __ai_audio_agent_msg_cb;
    agent_cbs.ai_agent_event_cb = __ai_audio_agent_event_cb;

    TUYA_CALL_ERR_RETURN(ai_audio_agent_init(&agent_cbs));
    sg_ai_audio_inform_cb = cfg->inform_cb;

    return OPRT_OK;
}

/**
 * @brief Sets the volume for the audio module.
 * @param volume The volume level to set.
 * @return OPERATE_RET - OPRT_OK if the volume is set successfully, otherwise an error code.
 */
OPERATE_RET ai_audio_set_volume(uint8_t volume)
{
    OPERATE_RET rt = OPRT_OK;

    // kv storage
    TUYA_CALL_ERR_LOG(tal_kv_set(AI_AUDIO_SPEAK_VOLUME_KEY, &volume, sizeof(volume)));

    TDL_AUDIO_HANDLE_T audio_hdl = NULL;
    TUYA_CALL_ERR_RETURN(tdl_audio_find(AUDIO_DRIVER_NAME, &audio_hdl));
    TUYA_CALL_ERR_LOG(tdl_audio_volume_set(audio_hdl, volume));

    return rt;
}

/**
 * @brief Retrieves the current volume setting for the audio module.
 * @param None
 * @return uint8_t - The current volume level.
 */
uint8_t ai_audio_get_volume(void)
{
    OPERATE_RET rt = OPRT_OK;

    uint8_t volume = 0;
    uint8_t *value = NULL;
    size_t read_len = 0;

    // kv read
    TUYA_CALL_ERR_LOG(tal_kv_get(AI_AUDIO_SPEAK_VOLUME_KEY, &value, &read_len));
    if (OPRT_OK != rt || NULL == value) {
        PR_ERR("read volume failed");
        volume = 50;
    } else {
        volume = *value;
    }

    PR_DEBUG("get spk volume: %d", volume);

    if (value) {
        tal_kv_free(value);
        value = NULL;
    }

    return volume;
}

OPERATE_RET ai_audio_set_open(bool is_open)
{
    if (true == is_open) {
        ai_audio_input_enable_get_valid_data(true);
    } else {
        ai_audio_input_enable_get_valid_data(false);

        if (ai_audio_player_is_playing()) {
            PR_DEBUG("player is playing, stop it first");
            ai_audio_player_stop();
        }

        ai_audio_cloud_asr_set_idle(true);

        sg_is_chating = false;
    }

    return OPRT_OK;
}

OPERATE_RET ai_audio_manual_start_single_talk(void)
{
    if (sg_ai_audio_work_mode != AI_AUDIO_MODE_MANUAL_SINGLE_TALK) {
        return OPRT_COM_ERROR;
    }

    ai_audio_input_manual_open_get_valid_data(true);

    return OPRT_OK;
}

OPERATE_RET ai_audio_manual_stop_single_talk(void)
{
    if (sg_ai_audio_work_mode != AI_AUDIO_MODE_MANUAL_SINGLE_TALK) {
        return OPRT_COM_ERROR;
    }

    ai_audio_input_manual_open_get_valid_data(false);

    return OPRT_OK;
}
