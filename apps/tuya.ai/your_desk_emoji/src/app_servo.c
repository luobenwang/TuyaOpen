#include "tkl_pwm.h"
#include "tal_system.h"
#include "tal_log.h"

#include "app_servo.h"

#define SERVO_PWM_VERTICAL           TUYA_PWM_NUM_1
#define SERVO_PWM_HORIZONTAL         TUYA_PWM_NUM_3
#define SERVO_PWM_FREQ               50      // 50Hz
#define SERVO_MIN_DUTY               250     // 0°, duty = 0.5ms/20ms * cycle = 250
#define SERVO_MAX_DUTY               1250    // 180°, duty = 2.5ms/20ms * cycle = 1250
#define SERVO_PWM_CYCLE              10000   // tkl_pwm cycle = 10000
#define SERVO_STEP_COUNT             100     // Number of steps for smooth movement
#define SERVO_MOVE_TIME_MS           2000    // Total move time in ms

// 舵机动作角度常量
#define SERVO_ANGLE_UP           0
#define SERVO_ANGLE_DOWN         70
#define SERVO_ANGLE_CENTER_VERT  35
#define SERVO_ANGLE_CENTER_HORI  90
#define SERVO_ANGLE_LEFT         30
#define SERVO_ANGLE_RIGHT        150

// 维护水平和垂直两个舵机的当前角度
STATIC UINT_T s_servo_horizontal_angle = SERVO_ANGLE_CENTER_HORI;
STATIC UINT_T s_servo_vertical_angle   = SERVO_ANGLE_CENTER_VERT;

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

// 优化：增加参数，分别控制水平和垂直通道的角度
STATIC VOID app_servo_move_to(TUYA_PWM_NUM_E ch_id, UINT_T *p_angle, INT_T target_angle)
{
    INT_T start_angle = *p_angle;
    INT_T delta = target_angle - start_angle;
    INT_T abs_delta = delta > 0 ? delta : -delta;
    if (abs_delta == 0) return;

    UINT_T total_time = (SERVO_MOVE_TIME_MS * abs_delta) / 180;
    if (total_time == 0) {
        total_time = SERVO_MOVE_TIME_MS / SERVO_STEP_COUNT;
    } else if (total_time > SERVO_MOVE_TIME_MS) {
        total_time = SERVO_MOVE_TIME_MS;
    }

    UINT_T steps = total_time / (SERVO_MOVE_TIME_MS / SERVO_STEP_COUNT);
    if (steps == 0) steps = 1;
    UINT_T step_delay = total_time / steps;

    for (UINT_T i = 1; i <= steps; ++i) {
        FLOAT_T t = (FLOAT_T)i / steps;
        FLOAT_T ease = ease_in_out_cubic(t);
        INT_T cur_angle = start_angle + (INT_T)(delta * ease + 0.5f);
        UINT32_T duty = angle_to_duty(cur_angle);
        tkl_pwm_duty_set(ch_id, duty);
        tal_system_sleep(step_delay);
    }
    *p_angle = target_angle;
}

OPERATE_RET app_servo_init(VOID)
{
    OPERATE_RET rt = OPRT_OK;
    TUYA_PWM_BASE_CFG_T cfg_x = {
        .frequency = SERVO_PWM_FREQ,
        .duty = angle_to_duty(SERVO_ANGLE_CENTER_HORI), // 中位
        .polarity = TUYA_PWM_POSITIVE,
    };

    TUYA_PWM_BASE_CFG_T cfg_y = {
        .frequency = SERVO_PWM_FREQ,
        .duty = angle_to_duty(SERVO_ANGLE_CENTER_VERT), // 中位
        .polarity = TUYA_PWM_NEGATIVE,
    };

    // 初始化水平PWM
    TUYA_CALL_ERR_RETURN(tkl_pwm_init(SERVO_PWM_HORIZONTAL, &cfg_x));
    TUYA_CALL_ERR_RETURN(tkl_pwm_start(SERVO_PWM_HORIZONTAL));

    // 初始化垂直PWM
    TUYA_CALL_ERR_RETURN(tkl_pwm_init(SERVO_PWM_VERTICAL, &cfg_y));
    TUYA_CALL_ERR_RETURN(tkl_pwm_start(SERVO_PWM_VERTICAL));

    PR_DEBUG("Servo initialized on channels %d (horizontal) and %d (vertical) with frequency %dHz", 
        SERVO_PWM_HORIZONTAL, SERVO_PWM_VERTICAL, SERVO_PWM_FREQ);

    s_servo_horizontal_angle = SERVO_ANGLE_CENTER_VERT;
    s_servo_vertical_angle = SERVO_ANGLE_CENTER_VERT;

    return OPRT_OK;
}

// 向上（垂直0°）
VOID app_servo_up(VOID)
{
    PR_DEBUG("Moving vertical servo to %d degrees", SERVO_ANGLE_UP);
    app_servo_move_to(SERVO_PWM_VERTICAL, &s_servo_vertical_angle, SERVO_ANGLE_UP);
}

// 向下（垂直90°）
VOID app_servo_down(VOID)
{
    PR_DEBUG("Moving vertical servo to %d degrees", SERVO_ANGLE_DOWN);
    app_servo_move_to(SERVO_PWM_VERTICAL, &s_servo_vertical_angle, SERVO_ANGLE_DOWN);
}

// 垂直回正（90°）
VOID app_servo_center(VOID)
{
    PR_DEBUG("Moving vertical and horizontal servo to %d and %d degrees", SERVO_ANGLE_CENTER_VERT, SERVO_ANGLE_CENTER_HORI);
    app_servo_move_to(SERVO_PWM_VERTICAL, &s_servo_vertical_angle, SERVO_ANGLE_CENTER_VERT);
    app_servo_move_to(SERVO_PWM_HORIZONTAL, &s_servo_horizontal_angle, SERVO_ANGLE_CENTER_HORI);
}

// 向左（水平0°）
VOID app_servo_left(VOID)
{
    PR_DEBUG("Moving horizontal servo to %d degrees", SERVO_ANGLE_LEFT);
    app_servo_move_to(SERVO_PWM_HORIZONTAL, &s_servo_horizontal_angle, SERVO_ANGLE_LEFT);
}

// 向右（水平180°）
VOID app_servo_right(VOID)
{
    PR_DEBUG("Moving horizontal servo to %d degrees", SERVO_ANGLE_RIGHT);
    app_servo_move_to(SERVO_PWM_HORIZONTAL, &s_servo_horizontal_angle, SERVO_ANGLE_RIGHT);
}