#include "tkl_pwm.h"
#include "tal_system.h"
#include "tal_log.h"

#include "app_servo.h"

#define SERVO_PWM_CH         TUYA_PWM_NUM_3
#define SERVO_PWM_FREQ       50      // 50Hz
#define SERVO_MIN_DUTY       250     // 0°, duty = 0.5ms/20ms * cycle = 250
#define SERVO_MAX_DUTY       1250    // 180°, duty = 2.5ms/20ms * cycle = 1250
#define SERVO_PWM_CYCLE      10000   // tkl_pwm cycle = 10000
#define SERVO_STEP_COUNT     200     // Number of steps for smooth movement
#define SERVO_MOVE_TIME_MS   2000    // Total move time in ms

STATIC UINT_T s_servo_cur_angle = 90;     // Current angle, 0~180

STATIC UINT32_T angle_to_duty(INT_T angle)
{
    FLOAT_T pulse_ms = 1.0;

    // Clamp angle
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;

    pulse_ms = 0.5 + (angle / 180.0) * 2;
    
    // 转换为占空比数值（周期20ms对应10000单位，1ms=500单位）
    return (UINT32_T)(pulse_ms * 500);
}

STATIC FLOAT_T ease_in_out_cubic(FLOAT_T t)
{
    if (t < 0.5) {
        return 4 * t * t * t;
    }

    return (1 - 4 * (1 - t) * (1 - t) * (1 - t));
}

STATIC VOID app_servo_move_to(TUYA_PWM_NUM_E ch_id, INT_T target_angle)
{
    INT_T start_angle = s_servo_cur_angle;
    INT_T delta = target_angle - start_angle;
    INT_T abs_delta = delta > 0 ? delta : -delta;
    if (abs_delta == 0) return;

    // 180度对应2s，比例缩放
    UINT_T total_time = (SERVO_MOVE_TIME_MS * abs_delta) / 180;
    if (total_time == 0) {
        total_time = SERVO_MOVE_TIME_MS / SERVO_STEP_COUNT; // 至少1步
    } else if (total_time > SERVO_MOVE_TIME_MS) {
        total_time = SERVO_MOVE_TIME_MS;
    }

    UINT_T steps = total_time / (SERVO_MOVE_TIME_MS / SERVO_STEP_COUNT);
    if (steps == 0) {
        steps = 1;
    }
    UINT_T step_delay = total_time / steps;

    for (UINT_T i = 1; i <= steps; ++i) {
        FLOAT_T t = (FLOAT_T)i / steps;
        FLOAT_T ease = ease_in_out_cubic(t);
        INT_T cur_angle = start_angle + (INT_T)(delta * ease + 0.5f);
        UINT32_T duty = angle_to_duty(cur_angle);
        tkl_pwm_duty_set(ch_id, duty);
        tal_system_sleep(step_delay);
    }
    s_servo_cur_angle = target_angle;
}

OPERATE_RET app_servo_init(VOID)
{
    OPERATE_RET rt = OPRT_OK;
    TUYA_PWM_BASE_CFG_T cfg = {
        .frequency = SERVO_PWM_FREQ,
        .duty = angle_to_duty(90), // 中位
        .polarity = TUYA_PWM_POSITIVE,
    };
    rt = tkl_pwm_init(SERVO_PWM_CH, &cfg);
    if (rt != OPRT_OK) {
        PR_ERR("Servo PWM init failed: %d", rt);
        return rt;
    }
    rt = tkl_pwm_start(SERVO_PWM_CH);
    if (rt != OPRT_OK) {
        PR_ERR("Servo PWM start failed: %d", rt);
        return rt;
    }

    PR_DEBUG("Servo initialized on channel %d with frequency %dHz", SERVO_PWM_CH, SERVO_PWM_FREQ);

    s_servo_cur_angle = 90;

    return OPRT_OK;
}

// 向上（180°）
VOID app_servo_up(VOID)
{
    PR_DEBUG("Moving servo to 180 degrees");
    app_servo_move_to(SERVO_PWM_CH, 180);
}

// 向下（0°）
VOID app_servo_down(VOID)
{
    PR_DEBUG("Moving servo to 0 degrees");
    app_servo_move_to(SERVO_PWM_CH, 0);
}

// 回正（90°）
VOID app_servo_center(VOID)
{
    PR_DEBUG("Moving servo to 90 degrees");
    app_servo_move_to(SERVO_PWM_CH, 90);
}