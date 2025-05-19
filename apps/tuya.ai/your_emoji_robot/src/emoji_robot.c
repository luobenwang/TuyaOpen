#include "tuya_cloud_types.h"

#include "tal_api.h"
#include "tuya_config.h"
#include "tuya_iot.h"
#include "tuya_iot_dp.h"

#include "tkl_output.h"



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
                PR_DEBUG("value:%d", dp->dps[index].value.dp_enum);
                // tkl_pwm_duty_set(TUYA_PWM_NUM_0, MAX_DUTY);
                target_duty1 = MIN_DUTY;
                _report_status(3);
            }
            
            if (dp->dps[index].value.dp_enum == 1 ) //后退 向下 电机1 反转
            {
                PR_DEBUG("value:%d", dp->dps[index].value.dp_enum);
                // tkl_pwm_duty_set(TUYA_PWM_NUM_0, MIN_DUTY);
                target_duty1 = MAX_DUTY;
                _report_status(3);
            }

            if (dp->dps[index].value.dp_enum == 2 ) //前进 向上 电机1 正转
            {
                PR_DEBUG("value:%d", dp->dps[index].value.dp_enum);
                // tkl_pwm_duty_set(TUYA_PWM_NUM_0, MAX_DUTY);
                target_duty2 = MIN_DUTY;
                _report_status(3);
            }

            if (dp->dps[index].value.dp_enum == 3 ) //后退 向下 电机1 反转
            {
                PR_DEBUG("value:%d", dp->dps[index].value.dp_enum);
                // tkl_pwm_duty_set(TUYA_PWM_NUM_0, MIN_DUTY);
                target_duty2 = MAX_DUTY;
                _report_status(3);
                
            }

            if (dp->dps[index].value.dp_enum == 4 ) //左 电机2 回到中间位置
            {
                PR_DEBUG("value:%d", dp->dps[index].value.dp_enum);
                // tkl_pwm_duty_set(TUYA_PWM_NUM_0, MID_DUTY);
                target_duty1 = MID_DUTY;
                target_duty2 = MID_DUTY;
                _report_status(0);
            }


        
        }
  
        
        if (!tal_sw_timer_is_running(robot_timer_id1) && dp->dps[index].id == 5 &&(dp->dps[index].value.dp_enum == 0 ||  dp->dps[index].value.dp_enum == 1||  dp->dps[index].value.dp_enum == 4)) {
            tal_sw_timer_start(robot_timer_id1, 200, TAL_TIMER_ONCE);
            PR_DEBUG("tal_sw_timer_start robot_timer_id1");
        }

        if (!tal_sw_timer_is_running(robot_timer_id2) && dp->dps[index].id == 5 && (dp->dps[index].value.dp_enum == 2 ||  dp->dps[index].value.dp_enum == 3||  dp->dps[index].value.dp_enum == 4)) {
             PR_DEBUG("tal_sw_timer_start robot_timer_id2");
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
    

    tkl_pwm_init(TUYA_PWM_NUM_0, &pwm_cfg);
    tkl_pwm_start(TUYA_PWM_NUM_0);

    tkl_pwm_init(TUYA_PWM_NUM_1, &pwm_cfg);
    tkl_pwm_start(TUYA_PWM_NUM_1);

    return OPRT_OK;
}
