/**
 * @file tdl_touch_manage.h
 * @version 0.1
 * @date 2025-06-09
 */

#ifndef __TDL_TOUCH_MANAGE_H__
#define __TDL_TOUCH_MANAGE_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
************************macro define************************
***********************************************************/
typedef void*  TDL_TOUCH_HANDLE_T;

/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef struct {
    uint16_t x;
    uint16_t y;
}TDL_TOUCH_POS_T;


/***********************************************************
********************function declaration********************
***********************************************************/
TDL_TOUCH_HANDLE_T tdl_touch_find_dev(char *name);

OPERATE_RET tdl_touch_dev_open(TDL_TOUCH_HANDLE_T touch_hdl);

OPERATE_RET tdl_touch_dev_read(TDL_TOUCH_HANDLE_T touch_hdl, uint8_t max_num,\
                               TDL_TOUCH_POS_T *point, uint8_t *point_num);

OPERATE_RET tdl_touch_dev_close(TDL_TOUCH_HANDLE_T touch_hdl);

#ifdef __cplusplus
}
#endif

#endif /* __TDL_TOUCH_MANAGE_H__ */
