/**
 * @file example_pwm.c
 * @brief PWM driver example for Tuya IoT projects.
 *
 * This file provides an example implementation of a PWM (Pulse Width Modulation) driver using the Tuya SDK.
 * It demonstrates the configuration and usage of PWM for controlling a servo motor, showing the Otto robot
 * movements library. The example demonstrates various servo-based movements for a small humanoid robot.
 *
 * @copyright Copyright (c) 2021-2024 Tuya Inc. All Rights Reserved.
 *
 */

#include "tuya_cloud_types.h"
#include "tal_api.h"
#include "tkl_output.h"
#include "tkl_pwm.h"
#include "oscillator.h"
#include "otto_movements.h"

/***********************************************************
*************************micro define***********************
***********************************************************/
// Otto机器人舵机引脚定义
#define PIN_LEFT_LEG    TUYA_PWM_NUM_0
#define PIN_RIGHT_LEG   TUYA_PWM_NUM_1
#define PIN_LEFT_FOOT   TUYA_PWM_NUM_2
#define PIN_RIGHT_FOOT  TUYA_PWM_NUM_3

#define TASK_PWM_PRIORITY THREAD_PRIO_2
#define TASK_PWM_SIZE     4096

/***********************************************************
***********************typedef define***********************
***********************************************************/

/***********************************************************
***********************variable define**********************
***********************************************************/
static THREAD_HANDLE sg_pwm_handle;

/***********************************************************
***********************function define**********************
***********************************************************/

/**
 * @brief pwm task
 *
 * @param[in] param:Task parameters
 * @return none
 */
static void __example_pwm_task(void *param)
{
    OPERATE_RET rt = OPRT_OK;

    PR_NOTICE("开始初始化Otto机器人...");
    
    // 初始化Otto机器人
    otto_init(PIN_LEFT_LEG, PIN_RIGHT_LEG, PIN_LEFT_FOOT, PIN_RIGHT_FOOT);
    
    // 设置舵机微调值 (如果舵机零位不准确，可以在这里调整)
    otto_set_trims(0, 0, 0, 0);
    
    // 启用舵机速度限制，防止舵机运动过快
    otto_enable_servo_limit(SERVO_LIMIT_DEFAULT);
    
    // 回到初始位置
    otto_home();
    tal_system_sleep(1000);
    
    PR_NOTICE("Otto初始化完成，开始演示移动...");

    // 演示一系列动作
    PR_NOTICE("演示行走动作");
    otto_walk(4, 1000, FORWARD);  // 向前走4步
    tal_system_sleep(500);
    
    PR_NOTICE("演示转向动作");
    otto_turn(4, 1000, LEFT);  // 向左转4步
    tal_system_sleep(500);
    
    PR_NOTICE("演示摇摆动作");
    otto_swing(4, 1000, 20);  // 左右摇摆4次
    tal_system_sleep(500);
    
    PR_NOTICE("演示上下运动");
    otto_up_down(4, 1000, 20);  // 上下运动4次
    tal_system_sleep(500);
    
    PR_NOTICE("演示弯腰动作");
    otto_bend(2, 1000, LEFT);  // 向左弯腰2次
    tal_system_sleep(500);
    
    PR_NOTICE("演示颤抖动作");
    otto_jitter(4, 500, 20);  // 颤抖4次
    tal_system_sleep(500);
    
    // 演示其他动作
    PR_NOTICE("演示月球漫步");
    otto_moonwalker(4, 1000, 20, LEFT);
    tal_system_sleep(500);
    
    PR_NOTICE("演示跳跃");
    otto_jump(2, 1000);
    tal_system_sleep(500);
    
    PR_NOTICE("演示舵机自由移动");
    // 通过MoveServos可以直接控制所有舵机到指定位置
    int positions[SERVO_COUNT] = {110, 70, 120, 60};
    otto_move_servos(1000, positions);
    tal_system_sleep(1000);
    
    // 回到初始位置
    PR_NOTICE("回到初始位置");
    otto_home();
    tal_system_sleep(1000);
    
    PR_NOTICE("演示结束");

    tal_thread_delete(sg_pwm_handle);
    return;
}

/**
 * @brief user_main
 *
 * @return none
 */
void otto_robot_main()
{
    OPERATE_RET rt = OPRT_OK;

    /* basic init */
    // tal_log_init(TAL_LOG_LEVEL_DEBUG, 1024, (TAL_LOG_OUTPUT_CB)tkl_log_output);

    /* a pwm thread */
    THREAD_CFG_T pwm_param = {.priority = TASK_PWM_PRIORITY, .stackDepth = TASK_PWM_SIZE, .thrdname = "pwm_task"};
    TUYA_CALL_ERR_LOG(tal_thread_create_and_start(&sg_pwm_handle, NULL, NULL, __example_pwm_task, NULL, &pwm_param));

    return;
}

#if 0
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
#endif
