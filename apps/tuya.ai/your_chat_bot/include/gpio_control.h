/**
 * @file gpio_control.h
 * @author AI Assistant
 * @brief GPIO控制模块头文件，用于控制GPIO2的高电平脉冲输出
 * @version 1.0
 * @date 2024-01-01
 *
 * @copyright Copyright (c) tuya.inc 2024
 *
 */

#ifndef __GPIO_CONTROL_H__
#define __GPIO_CONTROL_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
***********************function define**********************
***********************************************************/

/**
 * @brief 初始化GPIO控制模块
 * 
 * @return OPERATE_RET 操作结果
 */
OPERATE_RET gpio_control_init(VOID_T);

/**
 * @brief 反初始化GPIO控制模块
 * 
 * @return OPERATE_RET 操作结果
 */
OPERATE_RET gpio_control_deinit(VOID_T);

/**
 * @brief 设置GPIO2为高电平，100ms后自动变低
 * 
 * @return OPERATE_RET 操作结果
 */
OPERATE_RET set_gpio2_high(VOID_T);

/**
 * @brief 设置GPIO2为低电平
 * 
 * @return OPERATE_RET 操作结果
 */
OPERATE_RET set_gpio2_low(VOID_T);

/**
 * @brief 设置GPIO3为高电平，100ms后自动变低
 * 
 * @return OPERATE_RET 操作结果
 */
OPERATE_RET set_gpio3_high(VOID_T);

/**
 * @brief 设置GPIO3为低电平
 * 
 * @return OPERATE_RET 操作结果
 */
OPERATE_RET set_gpio3_low(VOID_T);

#ifdef __cplusplus
}
#endif

#endif /* __GPIO_CONTROL_H__ */
