#ifndef __APP_SERVO_H__
#define __APP_SERVO_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

OPERATE_RET app_servo_init(VOID);
VOID app_servo_up(VOID);
VOID app_servo_down(VOID);
VOID app_servo_center(VOID);

#ifdef __cplusplus
}
#endif
#endif // __APP_SERVO_H__