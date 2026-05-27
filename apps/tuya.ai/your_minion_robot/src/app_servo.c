/**
 * @file app_servo.c
 * @brief MG90S 360-degree continuous rotation servo
 *
 * MG90S 360 servo on BK7258:
 *   PWM 50Hz, cycle=10000
 *   duty=500  → 1.0ms → CW full speed
 *   duty=750  → 1.5ms → stop
 *   duty=1000 → 2.0ms → CCW full speed
 *
 * DP5:
 *   0 - 顺时针转一圈 (360°)
 *   1 - 逆时针转一圈 (360°)
 *   2 - 顺时针转半圈 (180°)
 *   3 - 逆时针转半圈 (180°)
 *   4 - 右转90° (顺时针)
 *   5 - 左转90° (逆时针)
 *   6 - 回正
 *   7 - 跳舞模式 (舞步+表情联动)
 */
#include "app_servo.h"
#include "tkl_pwm.h"
#include "tal_log.h"
#include "tal_system.h"
#include "tal_thread.h"
#include "tal_mutex.h"
#include "ai_ui_manage.h"
#include "skill_emotion.h"

/***********************************************************
************************macro define************************
***********************************************************/
#define SERVO_PWM_FREQ      50
#define SERVO_PWM_CYCLE     10000

#define SERVO_DUTY_CW       500     // 顺时针(右转)
#define SERVO_DUTY_CCW      1000    // 逆时针(左转)
#define SERVO_DUTY_STOP     750     // 停止

/*
 * Motion time calibration for continuous-rotation (360°) servo.
 * Run time is linear: ms = degrees * MS_PER_90 / 90.
 *
 * Tuning: send a 360° command, measure actual rotation, then update:
 *   MS_PER_90_new = MS_PER_90_old * 360 / actual_degrees
 *
 * History on this hardware:
 *   3 ms/deg          -> 360° cmd moved ~270°
 *   360 ms/90 + 30 ms -> 360° cmd moved ~405°
 *   327 ms/90         -> ~340° (still short ~15-20°)
 *   344 ms/90         -> target ~360° (360/342 correction)
 */
#define SERVO_MS_PER_90_CW   344U
#define SERVO_MS_PER_90_CCW  344U

/***********************************************************
***********************variable define**********************
***********************************************************/
static TUYA_PWM_NUM_E sg_pwm_ch = TUYA_PWM_NUM_0;
static bool sg_inited = false;
static int16_t sg_current_angle = 0;

static volatile uint8_t sg_pending_cmd = 0xFF;
static volatile bool sg_has_cmd = false;
static THREAD_HANDLE sg_thread = NULL;
static MUTEX_HANDLE sg_mutex = NULL;

/***********************************************************
***********************function define**********************
***********************************************************/
static void __servo_run(uint32_t duty, uint32_t ms)
{
    TUYA_PWM_BASE_CFG_T cfg = {0};
    cfg.polarity   = TUYA_PWM_POSITIVE;
    cfg.count_mode = TUYA_PWM_CNT_UP;
    cfg.frequency  = SERVO_PWM_FREQ;
    cfg.cycle      = SERVO_PWM_CYCLE;
    cfg.duty       = duty;

    tkl_pwm_info_set(sg_pwm_ch, &cfg);
    tkl_pwm_start(sg_pwm_ch);

    uint32_t elapsed = 0;
    while (elapsed < ms) {
        if (sg_has_cmd) {
            break;
        }
        tal_system_sleep(20);
        elapsed += 20;
    }

    tkl_pwm_stop(sg_pwm_ch);
}

static void __servo_rotate_degrees(uint32_t duty, uint32_t degrees)
{
    uint32_t ms_per_90 = (duty == SERVO_DUTY_CW) ? SERVO_MS_PER_90_CW : SERVO_MS_PER_90_CCW;
    uint32_t ms = (degrees * ms_per_90 + 45U) / 90U;

    if (ms == 0U && degrees > 0U) {
        ms = 1U;
    }

    PR_DEBUG("servo: duty=%d, degrees=%d, ms=%d", duty, degrees, ms);
    __servo_run(duty, ms);
}

static void __servo_goto_origin(void)
{
    if (sg_current_angle == 0) {
        return;
    }

    int16_t diff = -sg_current_angle;
    if (diff > 180) {
        diff -= 360;
    } else if (diff < -180) {
        diff += 360;
    }

    uint32_t duty = (diff > 0) ? SERVO_DUTY_CW : SERVO_DUTY_CCW;
    uint32_t degrees = (diff > 0) ? diff : -diff;

    PR_DEBUG("servo goto 0: current=%d, diff=%d", sg_current_angle, diff);
    __servo_rotate_degrees(duty, degrees);

    if (!sg_has_cmd) {
        sg_current_angle = 0;
    }
}

static void __set_emotion(const char *emotion)
{
    ai_ui_disp_msg(AI_UI_DISP_EMOTION, (uint8_t *)emotion, strlen(emotion));
}

static void __servo_dance(void)
{
    PR_NOTICE("servo dance start!");

    // 1. 开心摇摆: 左右快速晃动
    __set_emotion(EMOJI_HAPPY);
    if (sg_has_cmd) return;
    __servo_rotate_degrees(SERVO_DUTY_CW, 45);
    if (sg_has_cmd) return;
    __servo_rotate_degrees(SERVO_DUTY_CCW, 90);
    if (sg_has_cmd) return;
    __servo_rotate_degrees(SERVO_DUTY_CW, 90);
    if (sg_has_cmd) return;
    __servo_rotate_degrees(SERVO_DUTY_CCW, 45);
    if (sg_has_cmd) return;

    tal_system_sleep(300);
    if (sg_has_cmd) return;

    // 2. 惊讶转圈
    __set_emotion(EMOJI_SURPRISE);
    if (sg_has_cmd) return;
    __servo_rotate_degrees(SERVO_DUTY_CW, 360);
    if (sg_has_cmd) return;

    tal_system_sleep(200);
    if (sg_has_cmd) return;

    // 3. 思考: 慢慢左转再右转
    __set_emotion(EMOJI_THINKING);
    if (sg_has_cmd) return;
    __servo_rotate_degrees(SERVO_DUTY_CCW, 60);
    if (sg_has_cmd) return;
    tal_system_sleep(500);
    if (sg_has_cmd) return;
    __servo_rotate_degrees(SERVO_DUTY_CW, 60);
    if (sg_has_cmd) return;

    tal_system_sleep(300);
    if (sg_has_cmd) return;

    // 4. 生气: 快速左右抖动
    __set_emotion(EMOJI_ANGRY);
    if (sg_has_cmd) return;
    for (int i = 0; i < 4 && !sg_has_cmd; i++) {
        __servo_rotate_degrees(SERVO_DUTY_CW, 30);
        if (sg_has_cmd) return;
        __servo_rotate_degrees(SERVO_DUTY_CCW, 30);
        if (sg_has_cmd) return;
    }

    tal_system_sleep(300);
    if (sg_has_cmd) return;

    // 5. 害怕: 逆时针快跑一圈
    __set_emotion(EMOJI_FEARFUL);
    if (sg_has_cmd) return;
    __servo_rotate_degrees(SERVO_DUTY_CCW, 360);
    if (sg_has_cmd) return;

    tal_system_sleep(200);
    if (sg_has_cmd) return;

    // 6. 爱心: 顺时针慢转半圈
    __set_emotion(EMOJI_TOUCH);
    if (sg_has_cmd) return;
    __servo_rotate_degrees(SERVO_DUTY_CW, 180);
    if (sg_has_cmd) return;

    tal_system_sleep(300);
    if (sg_has_cmd) return;

    // 7. 开心结束: 左右摇摆回正
    __set_emotion(EMOJI_HAPPY);
    if (sg_has_cmd) return;
    __servo_rotate_degrees(SERVO_DUTY_CCW, 90);
    if (sg_has_cmd) return;
    __servo_rotate_degrees(SERVO_DUTY_CW, 90);
    if (sg_has_cmd) return;

    // 回到自然表情
    __set_emotion(EMOJI_NEUTRAL);
    sg_current_angle = 0;

    PR_NOTICE("servo dance done!");
}

static void __servo_thread_func(void *arg)
{
    while (1) {
        if (!sg_has_cmd) {
            tal_system_sleep(10);
            continue;
        }

        tal_mutex_lock(sg_mutex);
        uint8_t cmd = sg_pending_cmd;
        sg_has_cmd = false;
        tal_mutex_unlock(sg_mutex);

        PR_DEBUG("servo cmd: %d", cmd);

        switch (cmd) {
        case 0: // 顺时针转一圈 360°
            __servo_rotate_degrees(SERVO_DUTY_CW, 360);
            break;
        case 1: // 逆时针转一圈 360°
            __servo_rotate_degrees(SERVO_DUTY_CCW, 360);
            break;
        case 2: // 顺时针转半圈 180°
            __servo_rotate_degrees(SERVO_DUTY_CW, 180);
            if (!sg_has_cmd) {
                sg_current_angle = (sg_current_angle + 180) % 360;
            }
            break;
        case 3: // 逆时针转半圈 180°
            __servo_rotate_degrees(SERVO_DUTY_CCW, 180);
            if (!sg_has_cmd) {
                sg_current_angle = (sg_current_angle - 180 + 360) % 360;
            }
            break;
        case 4: // 右转90° (顺时针)
            __servo_rotate_degrees(SERVO_DUTY_CW, 90);
            if (!sg_has_cmd) {
                sg_current_angle = (sg_current_angle + 90) % 360;
            }
            break;
        case 5: // 左转90° (逆时针)
            __servo_rotate_degrees(SERVO_DUTY_CCW, 90);
            if (!sg_has_cmd) {
                sg_current_angle = (sg_current_angle - 90 + 360) % 360;
            }
            break;
        case 6: // 回正
            __servo_goto_origin();
            break;
        case 7: // 跳舞模式
            __servo_dance();
            break;
        default:
            break;
        }
    }
}

OPERATE_RET app_servo_init(TUYA_PWM_NUM_E pwm_ch)
{
    OPERATE_RET ret;

    sg_pwm_ch = pwm_ch;

    TUYA_PWM_BASE_CFG_T cfg = {0};
    cfg.polarity   = TUYA_PWM_POSITIVE;
    cfg.count_mode = TUYA_PWM_CNT_UP;
    cfg.frequency  = SERVO_PWM_FREQ;
    cfg.cycle      = SERVO_PWM_CYCLE;
    cfg.duty       = SERVO_DUTY_STOP;

    ret = tkl_pwm_init(sg_pwm_ch, &cfg);
    if (ret != OPRT_OK) {
        PR_ERR("servo pwm init failed: %d", ret);
        return ret;
    }

    sg_inited = true;
    sg_current_angle = 0;

    tal_mutex_create_init(&sg_mutex);

    THREAD_CFG_T thrd_cfg = {0};
    thrd_cfg.stackDepth = 2048;
    thrd_cfg.priority = 5;
    thrd_cfg.thrdname = "servo";
    tal_thread_create_and_start(&sg_thread, NULL, NULL, __servo_thread_func, NULL, &thrd_cfg);

    PR_NOTICE("servo init ok (MG90S 360), pwm_ch=%d", pwm_ch);
    return OPRT_OK;
}

void app_servo_cmd(uint8_t cmd)
{
    if (!sg_inited) {
        return;
    }
    tal_mutex_lock(sg_mutex);
    sg_pending_cmd = cmd;
    sg_has_cmd = true;
    tal_mutex_unlock(sg_mutex);
}

OPERATE_RET app_servo_deinit(void)
{
    if (!sg_inited) {
        return OPRT_OK;
    }
    sg_inited = false;
    tkl_pwm_stop(sg_pwm_ch);
    return tkl_pwm_deinit(sg_pwm_ch);
}
