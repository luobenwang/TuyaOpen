/*
 * OttoNinja APP 舵机运动控制 - Tuya平台版本
 * 移植自: examples/App/OttoNinja_APP/OttoNinja_APP.ino
 */

#include <stdint.h>
#include <stdbool.h>
#include "tuya_cloud_types.h"
#include "tal_api.h"
#include "tkl_pwm.h"                    // Tuya PWM定义
#include "tkl_output.h"                 // Tuya GPIO定义
#include "otto_ninja_app_servo.h"      // 引脚定义和函数声明

// ==================== Tuya平台接口实现 ====================

#define SERVO_PWM_FREQUENCY    50      // 舵机PWM频率：50Hz（20ms周期）
#define SERVO_PWM_PERIOD_US    20000   // PWM周期：20000微秒（20ms）
#define MAX_SERVO_COUNT        7       // 最大舵机数量

// PWM通道状态管理
typedef struct {
    bool initialized;
    TUYA_PWM_NUM_E pwm_id;
} pwm_channel_state_t;

static pwm_channel_state_t pwm_channels[MAX_SERVO_COUNT];

/**
 * @brief 根据引脚号获取对应的PWM通道
 * 
 * 注意：SERVO_*_PIN的值已经是TUYA_PWM_NUM_*枚举值
 * 所以pin参数传入的值实际上就是PWM通道号，可以直接转换使用
 */
static TUYA_PWM_NUM_E pin_to_pwm_id(uint8_t pin)
{
    return (TUYA_PWM_NUM_E)pin;
}

/**
 * @brief 获取系统运行时间（毫秒）
 */
uint32_t get_millis(void)
{
    return tal_system_get_millisecond();
}

/**
 * @brief 延时函数（毫秒）
 * 
 * 注意：Tuya SDK (T5AI平台/bk_system) 已经提供了 delay_ms 函数
 * 为了避免重复定义链接错误，这里使用弱符号（weak）属性
 * 如果 SDK 提供了 delay_ms，链接器会优先使用 SDK 的版本
 * 如果 SDK 没有提供，则使用这里的实现
 */
__attribute__((weak)) void delay_ms(uint32_t ms)
{
    tal_system_sleep(ms);
}

/**
 * @brief 设置GPIO为输出模式
 * 注意：Tuya SDK的PWM初始化会自动配置GPIO，此函数为空实现
 */
void gpio_set_output(uint8_t pin)
{
    // Tuya SDK的PWM初始化会自动配置GPIO
}

/**
 * @brief 初始化PWM通道
 */
bool pwm_init(uint8_t pin, uint32_t freq_hz)
{
    TUYA_PWM_NUM_E pwm_id = pin_to_pwm_id(pin);
    
    if (pwm_id >= TUYA_PWM_NUM_MAX) {
        return false;
    }
    
    // 检查是否已经初始化
    for (int i = 0; i < MAX_SERVO_COUNT; i++) {
        if (pwm_channels[i].initialized && pwm_channels[i].pwm_id == pwm_id) {
            return true;  // 已经初始化
        }
    }
    
    // 配置PWM参数
    TUYA_PWM_BASE_CFG_T pwm_cfg = {
        .duty = 0,              // 初始占空比为0
        .frequency = freq_hz,   // 频率
        .polarity = TUYA_PWM_NEGATIVE,  // 极性
    };
    
    // 初始化PWM
    OPERATE_RET ret = tkl_pwm_init(pwm_id, &pwm_cfg);
    if (ret != OPRT_OK) {
        return false;
    }
    
    // 记录初始化状态
    for (int i = 0; i < MAX_SERVO_COUNT; i++) {
        if (!pwm_channels[i].initialized) {
            pwm_channels[i].initialized = true;
            pwm_channels[i].pwm_id = pwm_id;
            break;
        }
    }
    
    return true;
}

/**
 * @brief 设置PWM占空比（通过脉冲宽度）
 */
void pwm_set_pulse_width(uint8_t pin, uint16_t pulse_width_us)
{
    TUYA_PWM_NUM_E pwm_id = pin_to_pwm_id(pin);
    
    if (pwm_id >= TUYA_PWM_NUM_MAX) {
        return;
    }
    
    // 限制脉冲宽度范围
    if (pulse_width_us > SERVO_PWM_PERIOD_US) {
        pulse_width_us = SERVO_PWM_PERIOD_US;
    }
    
    // 将脉冲宽度（微秒）转换为占空比
    // Tuya SDK的duty范围是1-10000，对应0.01%-100%
    // duty = (pulse_width_us / SERVO_PWM_PERIOD_US) * 10000
    uint32_t duty = (pulse_width_us * 10000) / SERVO_PWM_PERIOD_US;
    
    // 确保duty在有效范围内（1-10000）
    if (duty < 1) {
        duty = 1;
    } else if (duty > 10000) {
        duty = 10000;
    }
    
    // 设置占空比
    tkl_pwm_duty_set(pwm_id, duty);
    
    // 确保PWM正在运行
    tkl_pwm_start(pwm_id);
}

/**
 * @brief 停止PWM输出
 */
void pwm_stop(uint8_t pin)
{
    TUYA_PWM_NUM_E pwm_id = pin_to_pwm_id(pin);
    
    if (pwm_id >= TUYA_PWM_NUM_MAX) {
        return;
    }
    
    // 停止PWM输出
    tkl_pwm_stop(pwm_id);
    
    // 可选：将占空比设置为0
    tkl_pwm_duty_set(pwm_id, 0);
}

/**
 * @brief 初始化平台接口
 * 在系统启动时调用一次，用于初始化PWM通道状态管理数组
 */
void platform_tuya_init(void)
{
    // 初始化PWM通道状态数组
    for (int i = 0; i < MAX_SERVO_COUNT; i++) {
        pwm_channels[i].initialized = false;
        pwm_channels[i].pwm_id = TUYA_PWM_NUM_MAX;
    }
}

// ==================== PWM参数 ====================
#define SERVO_MIN_PULSE        500
#define SERVO_MAX_PULSE        2500

// ==================== 校准参数 ====================
#define LFFWRS     20      // 左脚前进旋转速度
#define RFFWRS     20      // 右脚前进旋转速度
#define LFBWRS     20      // 左脚后退旋转速度
#define RFBWRS     20      // 右脚后退旋转速度

#define LA0        60      // 左腿站立位置
#define RA0        120     // 右腿站立位置
#define LA1        180     // 左腿滚动位置
#define RA1        0       // 右腿滚动位置
#define LATL       100     // 左腿左倾斜行走位置
#define RATL       175     // 右腿左倾斜行走位置
#define LATR       5       // 左腿右倾斜行走位置
#define RATR       80      // 右腿右倾斜行走位置

// ==================== 数据结构 ====================
// 舵机状态结构
typedef struct {
    uint8_t pin;           // GPIO引脚
    bool attached;          // 是否已连接
    uint16_t current_angle; // 当前角度
    uint16_t min_pulse;     // 最小脉冲宽度（微秒）
    uint16_t max_pulse;     // 最大脉冲宽度（微秒）
} servo_t;

// ==================== 全局变量 ====================
// 全局舵机对象
static servo_t servos[7];

// 时间控制变量
static uint32_t currentmillis1 = 0;

// ==================== 工具函数 ====================

/**
 * 角度转换为PWM脉冲宽度（微秒）
 */
static uint16_t angle_to_pulse(uint16_t angle, uint16_t min_pulse, uint16_t max_pulse)
{
    if (angle > 180) angle = 180;
    return min_pulse + (angle * (max_pulse - min_pulse)) / 180;
}

/**
 * 获取舵机对象索引
 */
static int get_servo_index(uint8_t pin)
{
    for (int i = 0; i < 7; i++) {
        if (servos[i].pin == pin) {
            return i;
        }
    }
    return -1;
}

// ==================== 舵机控制函数 ====================

/**
 * 连接舵机到指定引脚（对应Arduino的attach函数）
 */
void servo_attach(uint8_t pin, uint16_t min_pulse, uint16_t max_pulse)
{
    int idx = get_servo_index(pin);
    if (idx < 0) {
        // 查找空闲位置
        for (int i = 0; i < 7; i++) {
            if (servos[i].pin == 0xFF) {  // 0xFF表示未使用
                idx = i;
                break;
            }
        }
        if (idx < 0) return;  // 没有空闲位置
    }
    
    servos[idx].pin = pin;
    servos[idx].min_pulse = min_pulse;
    servos[idx].max_pulse = max_pulse;
    servos[idx].attached = true;
    servos[idx].current_angle = 90;  // 默认90度
    
    // 初始化PWM
    gpio_set_output(pin);
    pwm_init(pin, 50);  // 50Hz
}

/**
 * 设置舵机角度（对应Arduino的write函数）
 */
void servo_write(uint8_t pin, uint16_t angle)
{
    int idx = get_servo_index(pin);
    if (idx < 0 || !servos[idx].attached) {
        return;
    }
    
    servos[idx].current_angle = angle;
    uint16_t pulse_width = angle_to_pulse(angle, servos[idx].min_pulse, servos[idx].max_pulse);
    
    // 计算占空比（百分比）
    uint32_t duty_percent = (pulse_width * 100) / SERVO_PWM_PERIOD_US;
    
    // 打印角度和占空比信息
    PR_NOTICE("servo_write: pin=%d, angle=%d, pulse_width=%d us, duty_percent=%d%%", 
              pin, angle, pulse_width, duty_percent);
    
    //pwm_set_pulse_width(pin, pulse_width);

    TUYA_PWM_NUM_E pwm_id = pin_to_pwm_id(pin);
    
    if (pwm_id >= TUYA_PWM_NUM_MAX) {
        return;
    }
    
    // 限制脉冲宽度范围
    if (pulse_width > SERVO_PWM_PERIOD_US) {
        pulse_width = SERVO_PWM_PERIOD_US;
    }
    
    // 将脉冲宽度（微秒）转换为占空比
    // Tuya SDK的duty范围是1-10000，对应0.01%-100%
    // duty = (pulse_width_us / SERVO_PWM_PERIOD_US) * 10000
    uint32_t duty = (pulse_width * 10000) / SERVO_PWM_PERIOD_US;
    
    // 确保duty在有效范围内（1-10000）
    if (duty < 1) {
        duty = 1;
    } else if (duty > 10000) {
        duty = 10000;
    }
    
    // 设置占空比
    tkl_pwm_duty_set(pwm_id, duty);

    PR_NOTICE("servo_write: pin=%d, angle=%d, pulse_width=%d us, duty=%d%%", 
              pin, angle, pulse_width, duty);
    // 确保PWM正在运行
    tkl_pwm_start(pwm_id);
}

/**
 * 断开舵机连接（对应Arduino的detach函数）
 */
void servo_detach(uint8_t pin)
{
    int idx = get_servo_index(pin);
    if (idx < 0) return;
    
    pwm_stop(pin);
    servos[idx].attached = false;
}


// ==================== 初始化函数 ====================

/**
 * 初始化舵机控制系统
 */
 void servo_control_init(void)
 {
     // 初始化舵机数组
     for (int i = 0; i < 7; i++) {
         servos[i].pin = 0xFF;  // 0xFF表示未使用
         servos[i].attached = false;
         servos[i].current_angle = 90;
         servos[i].min_pulse = SERVO_MIN_PULSE;
         servos[i].max_pulse = SERVO_MAX_PULSE;
     }
     
     currentmillis1 = 0;
 }
 

// ==================== 机器人初始化函数 ====================
void ninja_init(void)
{
    // 连接所有舵机
    servo_attach(SERVO_LEFT_FOOT_PIN, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
    servo_attach(SERVO_RIGHT_FOOT_PIN, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
    servo_attach(SERVO_LEFT_LEG_PIN, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
    servo_attach(SERVO_RIGHT_LEG_PIN, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
    servo_attach(SERVO_LEFT_ARM_PIN, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
    servo_attach(SERVO_RIGHT_ARM_PIN, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
    servo_attach(SERVO_HEAD_PIN, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
    
    // 设置初始位置
    servo_write(SERVO_HEAD_PIN, 90);
    servo_write(SERVO_LEFT_ARM_PIN, 90);
    servo_write(SERVO_RIGHT_ARM_PIN, 90);
    delay_ms(300);
    
    // 设置腿部位置
    servo_write(SERVO_LEFT_FOOT_PIN, 90);
    servo_write(SERVO_RIGHT_FOOT_PIN, 90);
    servo_write(SERVO_LEFT_LEG_PIN, LA0);
    servo_write(SERVO_RIGHT_LEG_PIN, RA0);
    delay_ms(300);
    
    // 设置手臂位置
    servo_write(SERVO_LEFT_ARM_PIN, 180);
    servo_write(SERVO_RIGHT_ARM_PIN, 0);
    delay_ms(500);
    
    // 断开所有舵机（节省功耗）
    servo_detach(SERVO_LEFT_FOOT_PIN);
    servo_detach(SERVO_RIGHT_FOOT_PIN);
    servo_detach(SERVO_LEFT_LEG_PIN);
    servo_detach(SERVO_RIGHT_LEG_PIN);
    servo_detach(SERVO_LEFT_ARM_PIN);
    servo_detach(SERVO_RIGHT_ARM_PIN);
    servo_detach(SERVO_HEAD_PIN);
}

// ==================== 模式设置函数 ====================
void ninja_set_walk(void)
{
    // 手臂到中间位置
    servo_attach(SERVO_LEFT_ARM_PIN, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
    servo_attach(SERVO_RIGHT_ARM_PIN, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
    servo_write(SERVO_LEFT_ARM_PIN, 90);
    servo_write(SERVO_RIGHT_ARM_PIN, 90);
    delay_ms(200);
    servo_detach(SERVO_LEFT_ARM_PIN);
    servo_detach(SERVO_RIGHT_ARM_PIN);
    
    // 腿部到站立位置
    servo_attach(SERVO_LEFT_LEG_PIN, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
    servo_attach(SERVO_RIGHT_LEG_PIN, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
    servo_write(SERVO_LEFT_LEG_PIN, LA0);
    servo_write(SERVO_RIGHT_LEG_PIN, RA0);
    delay_ms(300);
    servo_detach(SERVO_LEFT_LEG_PIN);
    servo_detach(SERVO_RIGHT_LEG_PIN);
    
    // 手臂到最终位置
    servo_attach(SERVO_LEFT_ARM_PIN, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
    servo_attach(SERVO_RIGHT_ARM_PIN, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
    servo_write(SERVO_LEFT_ARM_PIN, 180);
    servo_write(SERVO_RIGHT_ARM_PIN, 0);
    delay_ms(300);
    servo_detach(SERVO_LEFT_ARM_PIN);
    servo_detach(SERVO_RIGHT_ARM_PIN);
}

void ninja_set_roll(void)
{
    // 手臂到中间位置
    servo_attach(SERVO_LEFT_ARM_PIN, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
    servo_attach(SERVO_RIGHT_ARM_PIN, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
    servo_write(SERVO_LEFT_ARM_PIN, 90);
    servo_write(SERVO_RIGHT_ARM_PIN, 90);
    delay_ms(200);
    servo_detach(SERVO_LEFT_ARM_PIN);
    servo_detach(SERVO_RIGHT_ARM_PIN);
    
    // 腿部到滚动位置
    servo_attach(SERVO_LEFT_LEG_PIN, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
    servo_attach(SERVO_RIGHT_LEG_PIN, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
    servo_write(SERVO_LEFT_LEG_PIN, LA1);
    servo_write(SERVO_RIGHT_LEG_PIN, RA1);
    delay_ms(300);
    servo_detach(SERVO_LEFT_LEG_PIN);
    servo_detach(SERVO_RIGHT_LEG_PIN);
    
    // 手臂到最终位置
    servo_attach(SERVO_LEFT_ARM_PIN, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
    servo_attach(SERVO_RIGHT_ARM_PIN, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
    servo_write(SERVO_LEFT_ARM_PIN, 180);
    servo_write(SERVO_RIGHT_ARM_PIN, 0);
    delay_ms(300);
    servo_detach(SERVO_LEFT_ARM_PIN);
    servo_detach(SERVO_RIGHT_ARM_PIN);
}

// ==================== 停止函数 ====================
void ninja_stop(void)
{
    servo_detach(SERVO_LEFT_FOOT_PIN);
    servo_detach(SERVO_RIGHT_FOOT_PIN);
    servo_detach(SERVO_LEFT_LEG_PIN);
    servo_detach(SERVO_RIGHT_LEG_PIN);
}

void ninja_walk_stop(void)
{
    servo_write(SERVO_LEFT_FOOT_PIN, 90);
    servo_write(SERVO_RIGHT_FOOT_PIN, 90);
    servo_write(SERVO_LEFT_LEG_PIN, LA0);
    servo_write(SERVO_RIGHT_LEG_PIN, RA0);
}

void ninja_roll_stop(void)
{
    servo_write(SERVO_LEFT_FOOT_PIN, 90);
    servo_write(SERVO_RIGHT_FOOT_PIN, 90);
    servo_detach(SERVO_LEFT_FOOT_PIN);
    servo_detach(SERVO_RIGHT_FOOT_PIN);
}

// ==================== 行走控制函数 ====================
void ninja_walk_forward(void)
{
    // 直行前进（joystick_x=0, joystick_y=50）
    int lt = 450;  // 直行时左右转时间相等
    int rt = 450;
    
    // 计算时间间隔
    int interval1 = 250;
    int interval2 = 250 + rt;
    int interval3 = 250 + rt + 250;
    int interval4 = 250 + rt + 250 + lt;
    int interval5 = 250 + rt + 250 + lt + 50;
    
  
    //PR_NOTICE("ninja_walk_forward: interval1=%d, interval2=%d, interval3=%d, interval4=%d, interval5=%d", interval1, interval2, interval3, interval4, interval5);
    
    // 重置循环计时器
    if (get_millis() > currentmillis1 + interval5) {
        currentmillis1 = get_millis();
        //PR_NOTICE("ninja_walk_forward 1: currentmillis1=%ld", currentmillis1);
    }
    

    
    // 阶段1: 设置腿部到右倾斜位置
    if (get_millis() - currentmillis1 <= interval1) {
        PR_NOTICE("ninja_walk_forward 1: get_millis()=%ld", get_millis());
        servo_attach(SERVO_LEFT_LEG_PIN, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
        servo_attach(SERVO_RIGHT_LEG_PIN, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
        servo_attach(SERVO_RIGHT_FOOT_PIN, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
        servo_attach(SERVO_LEFT_FOOT_PIN, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
        servo_write(SERVO_LEFT_LEG_PIN, LATR);
        servo_write(SERVO_RIGHT_LEG_PIN, RATR);
    }


    // 阶段2: 右脚旋转
    if ((get_millis() - currentmillis1 >= interval1) && (get_millis() - currentmillis1 <= interval2)) {
        PR_NOTICE("ninja_walk_forward 2: get_millis()=%ld", get_millis());
        servo_write(SERVO_RIGHT_FOOT_PIN, 90 - RFFWRS);
        
    }

   
    //return;
    
    // 阶段3: 右脚停止，设置腿部到左倾斜位置
    if ((get_millis() - currentmillis1 >= interval2) && (get_millis() - currentmillis1 <= interval3)) {
        PR_NOTICE("ninja_walk_forward 3: get_millis()=%ld", get_millis());
        servo_detach(SERVO_RIGHT_FOOT_PIN);
        servo_write(SERVO_LEFT_LEG_PIN, LATL);
        servo_write(SERVO_RIGHT_LEG_PIN, RATL);
       
    }
    
    // 阶段4: 左脚旋转
    if ((get_millis() - currentmillis1 >= interval3) && (get_millis() - currentmillis1 <= interval4)) {
        PR_NOTICE("ninja_walk_forward 4: get_millis()=%ld", get_millis());
        servo_write(SERVO_LEFT_FOOT_PIN, 90 + LFFWRS);
    }
    
    // 阶段5: 左脚停止
    if ((get_millis() - currentmillis1 >= interval4) && (get_millis() - currentmillis1 <= interval5)) {
        PR_NOTICE("ninja_walk_forward 5: get_millis()=%ld", get_millis());
        servo_detach(SERVO_LEFT_FOOT_PIN);
    }
}

void ninja_walk_backward(void)
{
    // 直行后退（joystick_x=0, joystick_y=-50）
    int lt = 450;  // 直行时左右转时间相等
    int rt = 450;
    
    // 计算时间间隔
    int interval1 = 250;
    int interval2 = 250 + rt;
    int interval3 = 250 + rt + 250;
    int interval4 = 250 + rt + 250 + lt;
    int interval5 = 250 + rt + 250 + lt + 50;
    
    uint32_t current_time = get_millis();
    
    // 重置循环计时器
    if (current_time > currentmillis1 + interval5) {
        currentmillis1 = current_time;
    }
    
    uint32_t elapsed = current_time - currentmillis1;
    
    // 阶段1: 设置腿部到右倾斜位置
    if (elapsed <= interval1) {
        servo_attach(SERVO_LEFT_LEG_PIN, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
        servo_attach(SERVO_RIGHT_LEG_PIN, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
        servo_attach(SERVO_RIGHT_FOOT_PIN, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
        servo_attach(SERVO_LEFT_FOOT_PIN, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
        servo_write(SERVO_LEFT_LEG_PIN, LATR);
        servo_write(SERVO_RIGHT_LEG_PIN, RATR);
    }
    
    // 阶段2: 右脚旋转（后退方向）
    if (elapsed >= interval1 && elapsed <= interval2) {
        servo_write(SERVO_RIGHT_FOOT_PIN, 90 + RFBWRS);
    }
    
    // 阶段3: 右脚停止，设置腿部到左倾斜位置
    if (elapsed >= interval2 && elapsed <= interval3) {
        servo_detach(SERVO_RIGHT_FOOT_PIN);
        servo_write(SERVO_LEFT_LEG_PIN, LATL);
        servo_write(SERVO_RIGHT_LEG_PIN, RATL);
    }
    
    // 阶段4: 左脚旋转（后退方向）
    if (elapsed >= interval3 && elapsed <= interval4) {
        servo_write(SERVO_LEFT_FOOT_PIN, 90 - LFBWRS);
    }
    
    // 阶段5: 左脚停止
    if (elapsed >= interval4 && elapsed <= interval5) {
        servo_detach(SERVO_LEFT_FOOT_PIN);
    }
}

// ==================== 滚动模式控制 ====================
void ninja_roll_control(bool forward)
{
    // 直行滚动（joystick_x=0, joystick_y=50）
    // 连接脚部舵机
    servo_attach(SERVO_LEFT_FOOT_PIN, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
    servo_attach(SERVO_RIGHT_FOOT_PIN, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
    
    // 设置舵机角度（直行滚动）
    if (forward == true) {
        servo_write(SERVO_LEFT_FOOT_PIN, 135);
        servo_write(SERVO_RIGHT_FOOT_PIN, 45);
    }else{
        servo_write(SERVO_LEFT_FOOT_PIN, 45);
        servo_write(SERVO_RIGHT_FOOT_PIN, 135);
    }
    PR_NOTICE("ninja_roll_control: forward=%s", forward ? "true" : "false");
}

// ==================== 手臂控制函数 ====================
void ninja_left_arm_up(void)
{
    servo_attach(SERVO_LEFT_ARM_PIN, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
    servo_write(SERVO_LEFT_ARM_PIN, 90);
}

void ninja_left_arm_down(void)
{
    servo_write(SERVO_LEFT_ARM_PIN, 180);
}

void ninja_right_arm_up(void)
{
    servo_attach(SERVO_RIGHT_ARM_PIN, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
    servo_write(SERVO_RIGHT_ARM_PIN, 90);
}

void ninja_right_arm_down(void)
{
    servo_write(SERVO_RIGHT_ARM_PIN, 0);
}

// ==================== 演示函数 ====================
/**
 * 演示所有动作 - 依次执行每个动作函数
 */
void ninja_show(void)
{
    //delay_ms(500);
}
