/*
 * OttoNinja APP 使用示例
 * 展示如何使用C语言版本的舵机控制函数
 * 对应: examples/App/OttoNinja_APP/OttoNinja_APP.ino
 */

#include "../otto_ninja/otto_ninja_app_servo.h"
#include "../core/servo_control.h"
#include <stdint.h>
#include <stdbool.h>

// 平台相关函数声明（需要根据实际平台实现）
extern uint32_t get_millis(void);
extern void delay_ms(uint32_t ms);

// 假设这些是从遥控器或传感器获取的值
// 对应Arduino代码中的RemoteXY结构体
extern int8_t get_joystick_x(void);
extern int8_t get_joystick_y(void);
extern bool get_button_a(void);
extern bool get_button_b(void);
extern bool get_button_x(void);
extern bool get_button_y(void);

/**
 * 初始化示例（对应Arduino的setup函数）
 */
void setup_example(void)
{
    // 初始化舵机控制系统（servo_control.c中的函数）
    servo_control_init();
    
    // 机器人初始化到初始位置
    ninja_init();
    
    // 其他初始化代码...
    // Serial.begin(250000);  // 如果使用串口
    // RemoteXY_Init();       // 如果使用RemoteXY
}

/**
 * 主循环示例（对应Arduino的loop函数）
 * 
 * 这个函数展示了如何将Arduino代码中的loop()函数转换为C语言版本
 */
void loop_example(void)
{
    // 读取摇杆和按钮状态（对应RemoteXY_Handler()）
    int8_t joystick_x = get_joystick_x();
    int8_t joystick_y = get_joystick_y();
    bool button_a = get_button_a();
    bool button_b = get_button_b();
    bool button_x = get_button_x();
    bool button_y = get_button_y();
    
    // 主控制函数（包含所有舵机控制逻辑）
    ninja_main_control(joystick_x, joystick_y,
                       button_a, button_b, 
                       button_x, button_y);
    
    // 调试输出（对应Arduino的Serial.print）
    // printf("  X: %d  Y: %d  MC: %d\n", 
    //        joystick_x, joystick_y, ninja_get_mode());
}

/**
 * 完整的主函数示例
 */
int main(void)
{
    // 初始化
    setup_example();
    
    // 主循环
    while (1) {
        loop_example();
        
        // 可以添加小延时，避免CPU占用过高
        delay_ms(10);
    }
    
    return 0;
}

/**
 * 单独使用各个函数的示例
 */
void individual_function_examples(void)
{
    // 1. 初始化
    ninja_init();
    
    // 2. 设置模式
    ninja_set_walk();   // 切换到行走模式
    // 或
    ninja_set_roll();   // 切换到滚动模式
    
    // 3. 行走控制
    ninja_walk_forward(0, 50);    // 直行前进
    ninja_walk_forward(50, 50);   // 右转前进
    ninja_walk_backward(0, -50);  // 直行后退
    
    // 4. 滚动控制
    ninja_roll_control(0, 50);    // 直行滚动
    ninja_roll_control(50, 50);   // 右转滚动
    
    // 5. 手臂控制
    ninja_left_arm_up();
    ninja_left_arm_down();
    ninja_right_arm_up();
    ninja_right_arm_down();
    
    // 6. 停止
    ninja_walk_stop();   // 行走停止
    ninja_roll_stop();   // 滚动停止
    ninja_stop();        // 停止所有舵机
}

/**
 * 与Arduino代码的对应关系说明：
 * 
 * Arduino代码                          C语言代码
 * ============================================================
 * setup()中的舵机初始化部分    ->    ninja_init()
 * 
 * RemoteXY.button_X == HIGH    ->    button_x == true
 * RemoteXY.button_Y == HIGH    ->    button_y == true
 * RemoteXY.button_A == HIGH    ->    button_a == true
 * RemoteXY.button_B == HIGH    ->    button_b == true
 * 
 * RemoteXY.J_x                 ->    joystick_x
 * RemoteXY.J_y                 ->    joystick_y
 * 
 * NinjaSetWalk()               ->    ninja_set_walk()
 * NinjaSetRoll()               ->    ninja_set_roll()
 * NinjaWalkStop()              ->    ninja_walk_stop()
 * NinjaRollStop()              ->    ninja_roll_stop()
 * NinjaLeftArm()               ->    ninja_left_arm_up()
 * NinjaLeftArmDown()           ->    ninja_left_arm_down()
 * NinjaRightArm()              ->    ninja_right_arm_up()
 * NinjaRightArmDown()          ->    ninja_right_arm_down()
 * 
 * ModeCounter == 0             ->    ninja_get_mode() == 0
 * ModeCounter == 1             ->    ninja_get_mode() == 1
 * 
 * 前进行走逻辑                  ->    ninja_walk_forward()
 * 后退行走逻辑                  ->    ninja_walk_backward()
 * 滚动模式逻辑                  ->    ninja_roll_control()
 * 
 * 所有控制逻辑                  ->    ninja_main_control()
 */

