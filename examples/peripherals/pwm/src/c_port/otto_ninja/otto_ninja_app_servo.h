#ifndef OTTO_NINJA_APP_SERVO_H
#define OTTO_NINJA_APP_SERVO_H

#include <stdint.h>
#include <stdbool.h>
#include "tkl_pwm.h"  // Tuya PWM定义


// 注意：这些值实际上是TUYA_PWM_NUM_*枚举值，可以直接用作PWM通道号
#define SERVO_LEFT_LEG_PIN     TUYA_PWM_NUM_0      // 左踝舵机 -> TUYA_PWM_NUM_0
#define SERVO_RIGHT_LEG_PIN    TUYA_PWM_NUM_1      // 右踝舵机 -> TUYA_PWM_NUM_1
#define SERVO_LEFT_FOOT_PIN    TUYA_PWM_NUM_2      // 左脚舵机 -> TUYA_PWM_NUM_2
#define SERVO_RIGHT_FOOT_PIN   TUYA_PWM_NUM_3       //右脚舵机 -> TUYA_PWM_NUM_3
#define SERVO_LEFT_ARM_PIN     TUYA_PWM_NUM_4      // 左臂舵机 -> TUYA_PWM_NUM_4
#define SERVO_RIGHT_ARM_PIN    TUYA_PWM_NUM_7      // 右臂舵机 -> TUYA_PWM_NUM_7
#define SERVO_HEAD_PIN         TUYA_PWM_NUM_7      // 头部舵机 -> TUYA_PWM_NUM_7 (与右臂共用)

// ==================== 平台接口函数 ====================
/**
 * 获取系统运行时间（毫秒）
 */
uint32_t get_millis(void);

/**
 * 延时函数（毫秒）
 */
void delay_ms(uint32_t ms);

// ==================== 初始化 ====================
/**
 * 初始化Tuya平台接口
 * 在系统启动时调用一次，用于初始化PWM通道状态管理数组
 * 必须在调用其他PWM相关函数之前调用
 */
void platform_tuya_init(void);

/**
 * 初始化舵机控制系统
 */
void servo_control_init(void);

/**
 * 机器人初始化到初始位置
 * 对应Arduino setup()函数中的舵机初始化部分
 */
void ninja_init(void);

// ==================== 模式设置 ====================
/**
 * 设置行走模式
 * 对应Arduino的NinjaSetWalk()函数
 */
void ninja_set_walk(void);

/**
 * 设置滚动模式
 * 对应Arduino的NinjaSetRoll()函数
 */
void ninja_set_roll(void);

// ==================== 停止函数 ====================
/**
 * 停止所有舵机
 * 对应Arduino的NinjaStop()函数
 */
void ninja_stop(void);

/**
 * 行走停止
 * 对应Arduino的NinjaWalkStop()函数
 */
void ninja_walk_stop(void);

/**
 * 滚动停止
 * 对应Arduino的NinjaRollStop()函数
 */
void ninja_roll_stop(void);

// ==================== 行走控制 ====================
/**
 * 前进行走控制
 */
void ninja_walk_forward(void);

/**
 * 后退行走控制
 */
void ninja_walk_backward(void);

// ==================== 滚动模式控制 ====================
/**
 * 滚动模式控制
 */
void ninja_roll_control(void);

// ==================== 手臂控制 ====================
/**
 * 左臂抬起
 * 对应Arduino的NinjaLeftArm()函数
 */
void ninja_left_arm_up(void);

/**
 * 左臂放下
 * 对应Arduino的NinjaLeftArmDown()函数
 */
void ninja_left_arm_down(void);

/**
 * 右臂抬起
 * 对应Arduino的NinjaRightArm()函数
 */
void ninja_right_arm_up(void);

/**
 * 右臂放下
 * 对应Arduino的NinjaRightArmDown()函数
 */
void ninja_right_arm_down(void);

// ==================== 演示函数 ====================
/**
 * 演示所有动作 - 依次执行每个动作函数
 */
void ninja_show(void);

#endif // OTTO_NINJA_APP_SERVO_H

