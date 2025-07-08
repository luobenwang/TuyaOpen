/**
 * @file tdd_disp_st7735s.h
 * @version 0.1
 * @date 2025-03-12
 */

#ifndef __TDD_DISP_ST7789_H__
#define __TDD_DISP_ST7789_H__

#include "tuya_cloud_types.h"
#include "tdd_disp_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
************************macro define************************
***********************************************************/
#define ST7735S_CASET     0x2A // Column Address Set
#define ST7735S_RASET     0x2B // Row Address Set
#define ST7735S_RAMWR     0x2C

/***********************************************************
***********************typedef define***********************
***********************************************************/


/***********************************************************
********************function declaration********************
***********************************************************/
OPERATE_RET tdd_disp_qspi_st7735s_register(char *name, DISP_QSPI_DEVICE_CFG_T *dev_cfg);

#ifdef __cplusplus
}
#endif

#endif /* __TDD_DISP_ST7735S_H__ */
