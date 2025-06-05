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
#define SERVO_MOVE_TIME_MS           1000    // Total move time in ms

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

// 垂直回正（90°）
STATIC VOID app_servo_center(VOID)
{
    app_servo_move_to(SERVO_PWM_VERTICAL, &s_servo_vertical_angle, SERVO_ANGLE_CENTER_VERT);
    app_servo_move_to(SERVO_PWM_HORIZONTAL, &s_servo_horizontal_angle, SERVO_ANGLE_CENTER_HORI);
}

// 点头动作：回正->下(一半)->上(一半)，循环3次，最后回正
STATIC VOID app_servo_nod(VOID)
{
    UINT_T i;
    INT_T nod_down = (SERVO_ANGLE_CENTER_VERT + SERVO_ANGLE_DOWN) / 2;
    INT_T nod_up = (SERVO_ANGLE_CENTER_VERT + SERVO_ANGLE_UP) / 2;

    app_servo_move_to(SERVO_PWM_VERTICAL, &s_servo_vertical_angle, SERVO_ANGLE_CENTER_VERT);
    for (i = 0; i < 3; ++i) {
        app_servo_move_to(SERVO_PWM_VERTICAL, &s_servo_vertical_angle, nod_down);
        app_servo_move_to(SERVO_PWM_VERTICAL, &s_servo_vertical_angle, nod_up);
    }
    app_servo_move_to(SERVO_PWM_VERTICAL, &s_servo_vertical_angle, SERVO_ANGLE_CENTER_VERT);
}

// 顺时针动作：回正->同时向左和下->单独向上->单独向右->单独向下->回正
STATIC VOID app_servo_clockwise(VOID)
{
    // 回正
    app_servo_move_to(SERVO_PWM_VERTICAL, &s_servo_vertical_angle, SERVO_ANGLE_CENTER_VERT);
    app_servo_move_to(SERVO_PWM_HORIZONTAL, &s_servo_horizontal_angle, SERVO_ANGLE_CENTER_HORI);

    // 同时向左和下
    app_servo_move_to(SERVO_PWM_VERTICAL, &s_servo_vertical_angle, SERVO_ANGLE_DOWN);
    app_servo_move_to(SERVO_PWM_HORIZONTAL, &s_servo_horizontal_angle, SERVO_ANGLE_LEFT);

    // 单独向上
    app_servo_move_to(SERVO_PWM_VERTICAL, &s_servo_vertical_angle, SERVO_ANGLE_UP);

    // 单独向右
    app_servo_move_to(SERVO_PWM_HORIZONTAL, &s_servo_horizontal_angle, SERVO_ANGLE_RIGHT);

    // 单独向下
    app_servo_move_to(SERVO_PWM_VERTICAL, &s_servo_vertical_angle, SERVO_ANGLE_DOWN);

    // 回正
    app_servo_move_to(SERVO_PWM_VERTICAL, &s_servo_vertical_angle, SERVO_ANGLE_CENTER_VERT);
    app_servo_move_to(SERVO_PWM_HORIZONTAL, &s_servo_horizontal_angle, SERVO_ANGLE_CENTER_HORI);
}

// 逆时针动作：回正->同时向右和上->单独向下->单独向左->单独向上->回正
STATIC VOID app_servo_anticlockwise(VOID)
{
    // 回正
    app_servo_move_to(SERVO_PWM_VERTICAL, &s_servo_vertical_angle, SERVO_ANGLE_CENTER_VERT);
    app_servo_move_to(SERVO_PWM_HORIZONTAL, &s_servo_horizontal_angle, SERVO_ANGLE_CENTER_HORI);

    // 同时向右和上
    app_servo_move_to(SERVO_PWM_VERTICAL, &s_servo_vertical_angle, SERVO_ANGLE_UP);
    app_servo_move_to(SERVO_PWM_HORIZONTAL, &s_servo_horizontal_angle, SERVO_ANGLE_RIGHT);

    // 单独向下
    app_servo_move_to(SERVO_PWM_VERTICAL, &s_servo_vertical_angle, SERVO_ANGLE_DOWN);

    // 单独向左
    app_servo_move_to(SERVO_PWM_HORIZONTAL, &s_servo_horizontal_angle, SERVO_ANGLE_LEFT);

    // 单独向上
    app_servo_move_to(SERVO_PWM_VERTICAL, &s_servo_vertical_angle, SERVO_ANGLE_UP);

    // 回正
    app_servo_move_to(SERVO_PWM_VERTICAL, &s_servo_vertical_angle, SERVO_ANGLE_CENTER_VERT);
    app_servo_move_to(SERVO_PWM_HORIZONTAL, &s_servo_horizontal_angle, SERVO_ANGLE_CENTER_HORI);
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

VOID app_servo_move(SERVO_ACTION_E action)
{
    PR_DEBUG("servo action: %d", action);

    switch (action) {
        case SERVO_UP:
            app_servo_move_to(SERVO_PWM_VERTICAL, &s_servo_vertical_angle, SERVO_ANGLE_UP);
            break;
        case SERVO_DOWN:
            app_servo_move_to(SERVO_PWM_VERTICAL, &s_servo_vertical_angle, SERVO_ANGLE_DOWN);
            break;
        case SERVO_LEFT:
            app_servo_move_to(SERVO_PWM_HORIZONTAL, &s_servo_horizontal_angle, SERVO_ANGLE_LEFT);
            break;
        case SERVO_RIGHT:
            app_servo_move_to(SERVO_PWM_HORIZONTAL, &s_servo_horizontal_angle, SERVO_ANGLE_RIGHT);
            break;
        case SERVO_NOD:
            app_servo_nod();
            break;
        case SERVO_CLOCKWISE:
            app_servo_clockwise();
            break;
        case SERVO_ANTICLOCKWISE:
            app_servo_anticlockwise();
            break;
        case SERVO_CENTER:
            app_servo_center();
            break;
        default:
            PR_ERR("Unsupported servo action: %d", action);
            break;
    }
}