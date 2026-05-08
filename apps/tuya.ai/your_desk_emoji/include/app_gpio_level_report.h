/**
 * @file app_gpio_level_report.h
 * @brief GPIO level monitor and cloud upload (PROP_BOOL DP)
 * @version 1.0
 * @date 2026-04-30
 * @copyright Copyright (c) 2026 Tuya Inc. All Rights Reserved.
 */
#ifndef __APP_GPIO_LEVEL_REPORT_H__
#define __APP_GPIO_LEVEL_REPORT_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize GPIO as input and periodic sampling for level reporting
 * @return OPRT_OK on success
 */
OPERATE_RET app_gpio_level_report_init(void);

/**
 * @brief Report current GPIO level to cloud (e.g. after MQTT is connected)
 * @return OPRT_OK on success
 */
OPERATE_RET app_gpio_level_report_sync(void);

#ifdef __cplusplus
}
#endif

#endif /* __APP_GPIO_LEVEL_REPORT_H__ */
