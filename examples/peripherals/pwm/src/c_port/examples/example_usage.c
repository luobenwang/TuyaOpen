/*
 * 舵机控制使用示例
 * 展示如何使用C语言版本的舵机控制函数
 */

#include "../core/servo_control.h"
#include <stdint.h>
#include <stdbool.h>

// 平台相关函数声明（需要根据实际平台实现）
extern uint32_t get_millis(void);
extern void delay_ms(uint32_t ms);

// 假设这些是从遥控器或传感器获取的值
extern int8_t get_joystick_x(void);
extern int8_t get_joystick_y(void);
extern bool get_button_a(void);
extern bool get_button_b(void);
extern bool get_button_x(void);
extern bool get_button_y(void);

// 模式计数器
static int mode_counter = 0;  // 0=行走模式, 1=滚动模式

/**
 * 主循环示例（对应Arduino的loop函数）
 */
void main_loop(void)
{
    // 读取摇杆和按钮状态
    int8_t joystick_x = get_joystick_x();
    int8_t joystick_y = get_joystick_y();
    bool button_a = get_button_a();
    bool button_b = get_button_b();
    bool button_x = get_button_x();
    bool button_y = get_button_y();
    
    // 按钮X：切换到滚动模式
    if (button_x) {
        robot_set_roll();
        mode_counter = 1;
    }
    
    // 按钮Y：切换到行走模式
    if (button_y) {
        robot_set_walk();
        mode_counter = 0;
    }
    
    // 按钮A：左臂控制
    if (button_a) {
        robot_left_arm_up();
    } else {
        robot_left_arm_down();
    }
    
    // 按钮B：右臂控制
    if (button_b) {
        robot_right_arm_up();
    } else {
        robot_right_arm_down();
    }
    
    // 根据模式执行不同的运动控制
    if (mode_counter == 0) {
        // 行走模式
        // 如果摇杆在中心位置，停止
        if (joystick_x >= -10 && joystick_x <= 10 && 
            joystick_y >= -10 && joystick_y <= 10) {
            robot_walk_stop();
        }
        // 前进
        else if (joystick_y > 0) {
            robot_walk_forward(joystick_x, joystick_y);
        }
        // 后退
        else if (joystick_y < 0) {
            robot_walk_backward(joystick_x, joystick_y);
        }
    }
    else if (mode_counter == 1) {
        // 滚动模式
        robot_roll_control(joystick_x, joystick_y);
    }
}

/**
 * 初始化示例（对应Arduino的setup函数）
 */
void main_init(void)
{
    // 初始化舵机控制系统
    servo_control_init();
    
    // 机器人回到初始位置
    robot_home();
    
    // 其他初始化代码...
}

/**
 * 完整的主函数示例
 */
int main(void)
{
    // 初始化
    main_init();
    
    // 主循环
    while (1) {
        main_loop();
        
        // 可以添加小延时，避免CPU占用过高
        delay_ms(10);
    }
    
    return 0;
}

