/**
 * @file example_pwm.c
 * @brief PWM driver example for Tuya IoT projects.
 *
 * This file provides an example implementation of a PWM (Pulse Width Modulation) driver using the Tuya SDK.
 * It demonstrates the configuration and usage of PWM for controlling devices like LEDs or motors by varying the duty
 * cycle of the output signal. The example covers initializing a PWM channel, starting and stopping the PWM signal, and
 * dynamically changing the frequency and duty cycle of the PWM output.
 *
 * The PWM driver example aims to help developers understand how to use PWM in their Tuya IoT projects for applications
 * requiring precise control over the power delivered to devices. It includes detailed examples of setting up PWM
 * configurations, handling PWM operations, and integrating these functionalities within a multitasking environment.
 *
 * @note This example is designed to be adaptable to various Tuya IoT devices and platforms, showcasing fundamental PWM
 * operations critical for IoT device development.
 *
 * @copyright Copyright (c) 2021-2024 Tuya Inc. All Rights Reserved.
 *
 */

#include "tuya_cloud_types.h"
#include "tal_api.h"
#include "tkl_output.h"
#include "tkl_pwm.h"

// 包含舵机控制头文件
#include "c_port/otto_ninja/otto_ninja_app_servo.h"
 
 /***********************************************************
 *************************micro define***********************
 ***********************************************************/
 #define PWM_DUTY          5000 // 50% duty
 #define PWM_FREQUENCY     10000
 #define TASK_PWM_PRIORITY THREAD_PRIO_2
 #define TASK_PWM_SIZE     4096
 
 /***********************************************************
 ***********************typedef define***********************
 ***********************************************************/
 #define PWM_ID TUYA_PWM_NUM_0
 
 /***********************************************************
 ***********************variable define**********************
 ***********************************************************/
 static THREAD_HANDLE sg_pwm_handle;
 
 /***********************************************************
 ***********************function define**********************
 ***********************************************************/
 // 假设这些是从遥控器或传感器获取的值
 /**
  * @brief 设置摇杆X轴值
  * @param value: 摇杆X轴值
  * @return none
  */
int8_t joystick_x = 0;
void set_joystick_x(int8_t value){
    joystick_x = value;
}
/**
  * @brief 获取摇杆X轴值
  * @return 摇杆X轴值
  */
int8_t get_joystick_x(void){
    return joystick_x;
}
/**
  * @brief 设置摇杆Y轴值
  * @param value: 摇杆Y轴值
  * @return none
  */
int8_t joystick_y = 0;
void set_joystick_y(int8_t value){
    joystick_y = value;
}
/**
  * @brief 获取摇杆Y轴值
  * @return 摇杆Y轴值
  */
int8_t get_joystick_y(void){
    return joystick_y;
}

/**
  * @brief 设置按钮A值
  * @param value: 按钮A值
  * @return none
  */
bool button_a = false;
void set_button_a(bool value){
    button_a = value;
}
/**
  * @brief 获取按钮A值
  * @return 按钮A值
  */
bool get_button_a(void){
    return button_a;
}

bool button_b = false;
/**
  * @brief 设置按钮B值
  * @param value: 按钮B值
  * @return none
  */
void set_button_b(bool value){
    button_b = value;
}
/**
  * @brief 获取按钮B值
  * @return 按钮B值
  */
bool get_button_b(void){
    return button_b;
}

bool button_x = false;
/**
  * @brief 设置按钮X值
  * @param value: 按钮X值
  * @return none
  */
void set_button_x(bool value){
    button_x = value;
}
/**
  * @brief 获取按钮X值
  * @return 按钮X值
  */
bool get_button_x(void){
    return button_x;
}

bool button_y = false;
/**
  * @brief 设置按钮Y值
  * @param value: 按钮Y值
  * @return none
  */
void set_button_y(bool value){
    button_y = value;
}
/**
  * @brief 获取按钮Y值
  * @return 按钮Y值
  */
bool get_button_y(void){
    return button_y;
}

static int mode_counter = 0;  // 0=行走模式, 1=滚动模式
void set_mode_counter(int value){
    mode_counter = value;
}
int get_mode_counter(void){
    return mode_counter;
}
/**
 * @brief pwm task - 舵机运动控制演示
 *
 * @param[in] param:Task parameters
 * @return none
 */
static void __example_pwm_task(void *param)
{
    PR_NOTICE("=== OttoNinja Servo Control Task Start ===");

    // 1. 初始化Tuya平台接口
    platform_tuya_init();
    PR_NOTICE("Platform initialized");

    // // 2. 初始化舵机控制系统
   //  servo_control_init();
    // PR_NOTICE("Servo control system initialized");

    //     // 1. 初始化
   // ninja_init();
    // delay_ms(1000);

    //     // 2. 设置行走模式
       // ninja_set_walk();
    //     delay_ms(500);

            // // 11. 设置滚动模式
           // PR_NOTICE("Setting roll mode");
   //ninja_set_roll();
    //delay_ms(500);

    // 3. 执行演示 - 依次执行所有动作
    PR_NOTICE("Starting motion show...");
    //ninja_walk_forward();
    ninja_show();

   // int forwoard = 0;
    //bool forward = true;
    main_init();
    set_button_x(true); // 设置按钮X为false,不切换到滚动模式
    set_button_y(false); // 设置按钮Y为true,切换到行走模式
    set_joystick_x(0); // 设置摇杆X轴值为0
    set_joystick_y(-100); // 设置摇杆Y轴值为50,前进
    set_button_a(false); // 设置按钮A为false,不控制左臂
    set_button_b(false); // 设置按钮B为false,不控制右臂
        // bool button_a = get_button_a();
    // bool button_b = get_button_b();
    bool button_x = get_button_x();
    bool button_y = get_button_y();
    
    // // 按钮X：切换到滚动模式
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

    while (1) {
       
        //PR_NOTICE("Motion show completed, restarting...");
        //ninja_walk_forward();
        //ninja_walk_forward_test(0, 50);
      //  ninja_walk_backward();
       main_loop();
        tal_system_sleep(10);  // 等待2秒后重新开始
        // ninja_roll_control(forward);
        // forward = !forward;
    }

    PR_NOTICE("PWM task is finished, will delete");
    tal_thread_delete(sg_pwm_handle);
    return;
}
 
 /**
  * @brief user_main
  *
  * @return none
  */
 void user_main(void)
 {
     OPERATE_RET rt = OPRT_OK;
 
     /* basic init */
     tal_log_init(TAL_LOG_LEVEL_DEBUG, 1024, (TAL_LOG_OUTPUT_CB)tkl_log_output);
 
     PR_NOTICE("Application information:");
     PR_NOTICE("Project name:        %s", PROJECT_NAME);
     PR_NOTICE("App version:         %s", PROJECT_VERSION);
     PR_NOTICE("Compile time:        %s", __DATE__);
     PR_NOTICE("TuyaOpen version:    %s", OPEN_VERSION);
     PR_NOTICE("TuyaOpen commit-id:  %s", OPEN_COMMIT);
     PR_NOTICE("Platform chip:       %s", PLATFORM_CHIP);
     PR_NOTICE("Platform board:      %s", PLATFORM_BOARD);
     PR_NOTICE("Platform commit-id:  %s", PLATFORM_COMMIT);
 
     /* a pwm thread */
     THREAD_CFG_T pwm_param = {.priority = TASK_PWM_PRIORITY, .stackDepth = TASK_PWM_SIZE, .thrdname = "pwm_task"};
     TUYA_CALL_ERR_LOG(tal_thread_create_and_start(&sg_pwm_handle, NULL, NULL, __example_pwm_task, NULL, &pwm_param));
 
     return;
 }
 
 /**
  * @brief main
  *
  * @param argc
  * @param argv
  * @return void
  */
 #if OPERATING_SYSTEM == SYSTEM_LINUX
 void main(int argc, char *argv[])
 {
     user_main();
 
     while (1) {
        
         tal_system_sleep(500);
     }
 }
 #else
 
 /* Tuya thread handle */
 static THREAD_HANDLE ty_app_thread = NULL;
 
 /**
  * @brief  task thread
  *
  * @param[in] arg:Parameters when creating a task
  * @return none
  */
 static void tuya_app_thread(void *arg)
 {
     user_main();
 
     tal_thread_delete(ty_app_thread);
     ty_app_thread = NULL;
 }
 
 void tuya_app_main(void)
 {
     THREAD_CFG_T thrd_param = {4096, 4, "tuya_app_main"};
     tal_thread_create_and_start(&ty_app_thread, NULL, NULL, tuya_app_thread, NULL, &thrd_param);
 }
 #endif