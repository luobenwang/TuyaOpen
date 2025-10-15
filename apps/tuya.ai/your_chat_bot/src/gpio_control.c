/**
 * @file gpio_control.c
 * @author AI Assistant
 * @brief GPIO控制模块，用于控制GPIO2的高电平脉冲输出
 * @version 1.0
 * @date 2024-01-01
 *
 * @copyright Copyright (c) tuya.inc 2024
 *
 */

#include "tuya_cloud_types.h"
#include "tkl_gpio.h"
#include "tal_log.h"
#include "tal_system.h"
#include "tal_sw_timer.h"

/***********************************************************
***********************variable define**********************
***********************************************************/

/* GPIO引脚定义 */
#define GPIO_PIN_2    TUYA_GPIO_NUM_2
#define GPIO_PIN_3    TUYA_GPIO_NUM_3

/* 脉冲持续时间(毫秒) */
#define GPIO_PULSE_DURATION_MS    100

/* GPIO控制状态 */
STATIC BOOL_T sg_gpio_initialized = FALSE;

/* GPIO定时器ID */
STATIC TIMER_ID sg_gpio2_timer = NULL;
STATIC TIMER_ID sg_gpio3_timer = NULL;

/***********************************************************
***********************function define**********************
***********************************************************/

/**
 * @brief GPIO2定时器回调函数
 * 
 * @param[in] timer_id 定时器ID
 * @param[in] arg 参数
 */
STATIC VOID_T __gpio2_timer_callback(TIMER_ID timer_id, VOID_T *arg)
{
    /* 设置GPIO2为低电平 */
    tkl_gpio_write(GPIO_PIN_2, TUYA_GPIO_LEVEL_LOW);
    
    /* 停止并删除定时器 */
    tal_sw_timer_stop(timer_id);
    tal_sw_timer_delete(timer_id);
    sg_gpio2_timer = NULL;
    
    PR_DEBUG("GPIO2 set to low after 100ms");
}

/**
 * @brief GPIO3定时器回调函数
 * 
 * @param[in] timer_id 定时器ID
 * @param[in] arg 参数
 */
STATIC VOID_T __gpio3_timer_callback(TIMER_ID timer_id, VOID_T *arg)
{
    /* 设置GPIO3为低电平 */
    tkl_gpio_write(GPIO_PIN_3, TUYA_GPIO_LEVEL_LOW);
    
    /* 停止并删除定时器 */
    tal_sw_timer_stop(timer_id);
    tal_sw_timer_delete(timer_id);
    sg_gpio3_timer = NULL;
    
    PR_DEBUG("GPIO3 set to low after 100ms");
}

/**
 * @brief 初始化单个GPIO引脚
 * 
 * @param[in] pin GPIO引脚号
 * @return OPERATE_RET 操作结果
 */
STATIC OPERATE_RET __gpio_init_single(TUYA_GPIO_NUM_E pin)
{
    OPERATE_RET ret = OPRT_OK;
    TUYA_GPIO_BASE_CFG_T gpio_cfg = {
        .mode = TUYA_GPIO_PUSH_PULL,
        .direct = TUYA_GPIO_OUTPUT,
        .level = TUYA_GPIO_LEVEL_LOW
    };
    
    ret = tkl_gpio_init(pin, &gpio_cfg);
    if (ret != OPRT_OK) {
        PR_ERR("GPIO %d init failed, ret:%d", pin, ret);
        return ret;
    }
    
    PR_DEBUG("GPIO %d initialized successfully", pin);
    return OPRT_OK;
}


/**
 * @brief 初始化GPIO控制模块
 * 
 * @return OPERATE_RET 操作结果
 */
OPERATE_RET gpio_control_init(VOID_T)
{
    OPERATE_RET ret = OPRT_OK;
    
    if (sg_gpio_initialized) {
        PR_DEBUG("GPIO control already initialized");
        return OPRT_OK;
    }
    
    PR_DEBUG("GPIO control module init start");
    
    /* 初始化GPIO2引脚 */
    ret = __gpio_init_single(GPIO_PIN_2);
    if (ret != OPRT_OK) {
        PR_ERR("GPIO2 init failed");
        return ret;
    }
    
    /* 初始化GPIO3引脚 */
    ret = __gpio_init_single(GPIO_PIN_3);
    if (ret != OPRT_OK) {
        PR_ERR("GPIO3 init failed");
        return ret;
    }
    
    sg_gpio2_timer = NULL;
    sg_gpio3_timer = NULL;
    sg_gpio_initialized = TRUE;
    PR_DEBUG("GPIO control module init success");
    return OPRT_OK;
}

/**
 * @brief 反初始化GPIO控制模块
 * 
 * @return OPERATE_RET 操作结果
 */
OPERATE_RET gpio_control_deinit(VOID_T)
{
    OPERATE_RET ret = OPRT_OK;
    
    if (!sg_gpio_initialized) {
        PR_DEBUG("GPIO control not initialized");
        return OPRT_OK;
    }
    
    PR_DEBUG("GPIO control module deinit start");
    
    /* 停止GPIO2定时器并删除 */
    if (sg_gpio2_timer != NULL) {
        tal_sw_timer_stop(sg_gpio2_timer);
        tal_sw_timer_delete(sg_gpio2_timer);
        sg_gpio2_timer = NULL;
    }
    
    /* 停止GPIO3定时器并删除 */
    if (sg_gpio3_timer != NULL) {
        tal_sw_timer_stop(sg_gpio3_timer);
        tal_sw_timer_delete(sg_gpio3_timer);
        sg_gpio3_timer = NULL;
    }
    
    /* 设置GPIO2为低电平 */
    tkl_gpio_write(GPIO_PIN_2, TUYA_GPIO_LEVEL_LOW);
    
    /* 设置GPIO3为低电平 */
    tkl_gpio_write(GPIO_PIN_3, TUYA_GPIO_LEVEL_LOW);
    
    /* 反初始化GPIO2 */
    ret = tkl_gpio_deinit(GPIO_PIN_2);
    if (ret != OPRT_OK) {
        PR_ERR("GPIO2 deinit failed, ret:%d", ret);
    }
    
    /* 反初始化GPIO3 */
    ret = tkl_gpio_deinit(GPIO_PIN_3);
    if (ret != OPRT_OK) {
        PR_ERR("GPIO3 deinit failed, ret:%d", ret);
    }
    
    sg_gpio_initialized = FALSE;
    PR_DEBUG("GPIO control module deinit success");
    return OPRT_OK;
}

/**
 * @brief 设置GPIO2为高电平，100ms后自动变低
 * 
 * @return OPERATE_RET 操作结果
 */
OPERATE_RET set_gpio2_high(VOID_T)
{
    OPERATE_RET ret = OPRT_OK;
    
    if (!sg_gpio_initialized) {
        PR_ERR("GPIO control not initialized");
        return OPRT_RESOURCE_NOT_READY;
    }
    
    /* 如果已有定时器在运行，先停止并删除 */
    if (sg_gpio2_timer != NULL) {
        tal_sw_timer_stop(sg_gpio2_timer);
        tal_sw_timer_delete(sg_gpio2_timer);
        sg_gpio2_timer = NULL;
    }
    
    /* 设置GPIO2为高电平 */
    ret = tkl_gpio_write(GPIO_PIN_2, TUYA_GPIO_LEVEL_HIGH);
    if (ret != OPRT_OK) {
        PR_ERR("GPIO2 set high failed, ret:%d", ret);
        return ret;
    }
    
    /* 创建定时器，100ms后自动变低 */
    ret = tal_sw_timer_create(__gpio2_timer_callback, NULL, &sg_gpio2_timer);
    if (ret != OPRT_OK) {
        PR_ERR("GPIO2 timer create failed, ret:%d", ret);
        return ret;
    }
    
    /* 启动定时器 */
    ret = tal_sw_timer_start(sg_gpio2_timer, GPIO_PULSE_DURATION_MS, TAL_TIMER_ONCE);
    if (ret != OPRT_OK) {
        PR_ERR("GPIO2 timer start failed, ret:%d", ret);
        tal_sw_timer_delete(sg_gpio2_timer);
        sg_gpio2_timer = NULL;
        return ret;
    }
    
    PR_DEBUG("GPIO2 set to high, will auto low after 100ms");
    return OPRT_OK;
}

/**
 * @brief 设置GPIO2为低电平
 * 
 * @return OPERATE_RET 操作结果
 */
OPERATE_RET set_gpio2_low(VOID_T)
{
    if (!sg_gpio_initialized) {
        PR_ERR("GPIO control not initialized");
        return OPRT_RESOURCE_NOT_READY;
    }
    
    /* 停止定时器 */
    if (sg_gpio2_timer != NULL) {
        tal_sw_timer_stop(sg_gpio2_timer);
        tal_sw_timer_delete(sg_gpio2_timer);
        sg_gpio2_timer = NULL;
    }
    
    return tkl_gpio_write(GPIO_PIN_2, TUYA_GPIO_LEVEL_LOW);
}

/**
 * @brief 设置GPIO3为高电平，100ms后自动变低
 * 
 * @return OPERATE_RET 操作结果
 */
OPERATE_RET set_gpio3_high(VOID_T)
{
    OPERATE_RET ret = OPRT_OK;
    
    if (!sg_gpio_initialized) {
        PR_ERR("GPIO control not initialized");
        return OPRT_RESOURCE_NOT_READY;
    }
    
    /* 如果已有定时器在运行，先停止并删除 */
    if (sg_gpio3_timer != NULL) {
        tal_sw_timer_stop(sg_gpio3_timer);
        tal_sw_timer_delete(sg_gpio3_timer);
        sg_gpio3_timer = NULL;
    }
    
    /* 设置GPIO3为高电平 */
    ret = tkl_gpio_write(GPIO_PIN_3, TUYA_GPIO_LEVEL_HIGH);
    if (ret != OPRT_OK) {
        PR_ERR("GPIO3 set high failed, ret:%d", ret);
        return ret;
    }
    
    /* 创建定时器，100ms后自动变低 */
    ret = tal_sw_timer_create(__gpio3_timer_callback, NULL, &sg_gpio3_timer);
    if (ret != OPRT_OK) {
        PR_ERR("GPIO3 timer create failed, ret:%d", ret);
        return ret;
    }
    
    /* 启动定时器 */
    ret = tal_sw_timer_start(sg_gpio3_timer, GPIO_PULSE_DURATION_MS, TAL_TIMER_ONCE);
    if (ret != OPRT_OK) {
        PR_ERR("GPIO3 timer start failed, ret:%d", ret);
        tal_sw_timer_delete(sg_gpio3_timer);
        sg_gpio3_timer = NULL;
        return ret;
    }
    
    PR_DEBUG("GPIO3 set to high, will auto low after 100ms");
    return OPRT_OK;
}

/**
 * @brief 设置GPIO3为低电平
 * 
 * @return OPERATE_RET 操作结果
 */
OPERATE_RET set_gpio3_low(VOID_T)
{
    if (!sg_gpio_initialized) {
        PR_ERR("GPIO control not initialized");
        return OPRT_RESOURCE_NOT_READY;
    }
    
    /* 停止定时器 */
    if (sg_gpio3_timer != NULL) {
        tal_sw_timer_stop(sg_gpio3_timer);
        tal_sw_timer_delete(sg_gpio3_timer);
        sg_gpio3_timer = NULL;
    }
    
    return tkl_gpio_write(GPIO_PIN_3, TUYA_GPIO_LEVEL_LOW);
}
