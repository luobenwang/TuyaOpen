/**
 * @file app_servo.h
 * @brief MG90S 360-degree servo control
 *
 * DP5枚举:
 *   0 - 顺时针转一圈 (360°)
 *   1 - 逆时针转一圈 (360°)
 *   2 - 顺时针转半圈 (180°)
 *   3 - 逆时针转半圈 (180°)
 *   4 - 右转90° (顺时针)
 *   5 - 左转90° (逆时针)
 *   6 - 回正
 *   7 - 跳舞模式 (舞步+表情联动)
 */
#ifndef __APP_SERVO_H__
#define __APP_SERVO_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

OPERATE_RET app_servo_init(TUYA_PWM_NUM_E pwm_ch);
void app_servo_cmd(uint8_t cmd);
OPERATE_RET app_servo_deinit(void);

#ifdef __cplusplus
}
#endif

#endif /* __APP_SERVO_H__ */
