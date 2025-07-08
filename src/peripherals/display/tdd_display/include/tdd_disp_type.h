/**
 * @file tdd_disp_type.h
 * @brief tdd_disp_type module is used to 
 * @version 0.1
 * @date 2025-05-28
 */

#ifndef __TDD_DISP_TYPE_H__
#define __TDD_DISP_TYPE_H__

#include "tuya_cloud_types.h"
#include "tdd_disp_sw_spi.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
************************macro define************************
***********************************************************/


/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef struct {
    TDD_DISP_SW_SPI_CFG_T    sw_spi_cfg;  // Software SPI configuration
    uint16_t                 width;
    uint16_t                 height;
	TUYA_DISPLAY_PIXEL_FMT_E pixel_fmt;
    TUYA_DISPLAY_ROTATION_E  rotation;
    TUYA_DISPLAY_BL_CTRL_T   bl;
    TUYA_DISPLAY_IO_CTRL_T   power;
}DISP_RGB_DEVICE_CFG_T;

typedef struct {
    uint16_t                 width;
    uint16_t                 height;
	TUYA_DISPLAY_PIXEL_FMT_E pixel_fmt;
    TUYA_DISPLAY_ROTATION_E  rotation;
    TUYA_GPIO_NUM_E          cs_pin;
    TUYA_GPIO_NUM_E          dc_pin;
    TUYA_GPIO_NUM_E          rst_pin;
    TUYA_SPI_NUM_E           port;
    uint32_t                 spi_clk;
    TUYA_DISPLAY_BL_CTRL_T   bl;
    TUYA_DISPLAY_IO_CTRL_T   power;
}DISP_SPI_DEVICE_CFG_T;

typedef struct {
    uint16_t                 width;
    uint16_t                 height;
	TUYA_DISPLAY_PIXEL_FMT_E pixel_fmt;
    TUYA_DISPLAY_ROTATION_E  rotation;
    TUYA_GPIO_NUM_E          cs_pin;
    TUYA_GPIO_NUM_E          dc_pin;
    TUYA_GPIO_NUM_E          rst_pin;
    TUYA_QSPI_NUM_E          port;
    uint32_t                 spi_clk;
    TUYA_DISPLAY_BL_CTRL_T   bl;
    TUYA_DISPLAY_IO_CTRL_T   power;
}DISP_QSPI_DEVICE_CFG_T;

typedef struct {
    uint16_t                 width;
    uint16_t                 height;
	TUYA_DISPLAY_PIXEL_FMT_E pixel_fmt;
    TUYA_DISPLAY_ROTATION_E  rotation;
    TUYA_GPIO_NUM_E          te_pin;
    TUYA_GPIO_IRQ_E          te_mode;
    uint32_t                 clk;
    uint8_t                  data_bits; // 8, 9, 16, 18, 24
    TUYA_DISPLAY_BL_CTRL_T   bl;
    TUYA_DISPLAY_IO_CTRL_T   power;
}DISP_MCU8080_DEVICE_CFG_T;

/***********************************************************
********************function declaration********************
***********************************************************/

#ifdef __cplusplus
}
#endif

#endif /* __TDD_DISP_TYPE_H__ */
