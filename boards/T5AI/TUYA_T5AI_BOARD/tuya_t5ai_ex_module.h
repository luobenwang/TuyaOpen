/**
 * @file tuya_t5ai_ex_module.h
 * @version 0.1
 * @date 2025-07-01
 */

#ifndef __TUYA_T5AI_EX_MODULE_H__
#define __TUYA_T5AI_EX_MODULE_H__

#include "tuya_cloud_types.h"

#if defined (TUYA_T5AI_BOARD_EX_MODULE_35565LCD) && (TUYA_T5AI_BOARD_EX_MODULE_35565LCD ==1)
#include "tdd_disp_ili9488.h"
#include "tdd_tp_gt1151.h"
#elif defined (TUYA_T5AI_BOARD_EX_MODULE_EYES) && (TUYA_T5AI_BOARD_EX_MODULE_EYES ==1)
//#include "tdd_disp_st7735s.h"
#include "tdd_disp_gc9d01.h"
#elif defined (TUYA_T5AI_BOARD_EX_MODULE_096_OLED) && (TUYA_T5AI_BOARD_EX_MODULE_096_OLED ==1)
#include "tdd_disp_ssd1306.h"
#endif

#if defined (ENABLE_EX_MODULE_CAMERA) && (ENABLE_EX_MODULE_CAMERA ==1)
#include "tdd_camera_gc2145.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
************************macro define************************
***********************************************************/
#if defined (TUYA_T5AI_BOARD_EX_MODULE_35565LCD) && (TUYA_T5AI_BOARD_EX_MODULE_35565LCD ==1)
#define BOARD_LCD_SW_SPI_CLK_PIN     TUYA_GPIO_NUM_49
#define BOARD_LCD_SW_SPI_CSX_PIN     TUYA_GPIO_NUM_48
#define BOARD_LCD_SW_SPI_SDA_PIN     TUYA_GPIO_NUM_50
#define BOARD_LCD_SW_SPI_DC_PIN      TUYA_GPIO_NUM_MAX
#define BOARD_LCD_SW_SPI_RST_PIN     TUYA_GPIO_NUM_53

#define BOARD_LCD_BL_TYPE            TUYA_DISP_BL_TP_PWM
#define BOARD_LCD_BL_PWM_ID          TUYA_PWM_NUM_7

#define BOARD_LCD_WIDTH              320
#define BOARD_LCD_HEIGHT             480
#define BOARD_LCD_PIXELS_FMT         TUYA_PIXEL_FMT_RGB565
#define BOARD_LCD_ROTATION           TUYA_DISPLAY_ROTATION_0

#define BOARD_LCD_POWER_PIN          TUYA_GPIO_NUM_MAX

#define BOARD_TP_I2C_PORT            TUYA_I2C_NUM_0
#define BOARD_TP_I2C_SCL_PIN         TUYA_GPIO_NUM_13
#define BOARD_TP_I2C_SDA_PIN         TUYA_GPIO_NUM_15

#elif defined (TUYA_T5AI_BOARD_EX_MODULE_EYES) && (TUYA_T5AI_BOARD_EX_MODULE_EYES ==1)
#define BOARD_LCD_BL_TYPE            TUYA_DISP_BL_TP_GPIO
#if defined(ENABLE_EYES_TWO_LCD_SAME) && (ENABLE_EYES_TWO_LCD_SAME == 1)
#define BOARD_LCD_BL_PIN             TUYA_GPIO_NUM_5
#define BOARD_LCD_BL_ACTIVE_LV       TUYA_GPIO_LEVEL_LOW
#else
#define BOARD_LCD_BL_PIN             TUYA_GPIO_NUM_47
#define BOARD_LCD_BL_ACTIVE_LV       TUYA_GPIO_LEVEL_HIGH
#endif

#define BOARD_LCD_WIDTH              160
#define BOARD_LCD_HEIGHT             160
#define BOARD_LCD_X_OFFSET           0
#define BOARD_LCD_Y_OFFSET           0
#define BOARD_LCD_PIXELS_FMT         TUYA_PIXEL_FMT_RGB565
#if defined(ENABLE_EYES_TWO_LCD_SAME) && (ENABLE_EYES_TWO_LCD_SAME == 1)
#define BOARD_LCD_ROTATION           TUYA_DISPLAY_ROTATION_0
#else
#define BOARD_LCD_ROTATION           TUYA_DISPLAY_ROTATION_180
#endif

#if defined(ENABLE_EYES_TWO_LCD_SAME) && (ENABLE_EYES_TWO_LCD_SAME == 1)
/*
SPI0 drives mirrored eye LCDs
        SCL        P14        SPI0 clock
        CS         P13        SPI0 chip select
        SDA        P16        SPI0 data
        RST        P19        screen reset
        DC         P17        data/command select
        BLK        P47        screen backlight
*/
#define BOARD_LCD_SPI_PORT           TUYA_SPI_NUM_0
#define BOARD_LCD_SPI_CLK            48000000
#define BOARD_LCD_SPI_CS_PIN         TUYA_GPIO_NUM_13
#define BOARD_LCD_SPI_DC_PIN         TUYA_GPIO_NUM_17
#define BOARD_LCD_SPI_RST_PIN        TUYA_GPIO_NUM_19
#else
/*
QSPI0 drives main LCD
        SCL        P22        QSPI0 clock
        CS         P23        QSPI0 chip select
        SDA        P24        QSPI0 data
        RST        P8         screen reset
        DC         P7         data/command select
        BLK        P47        screen backlight
*/
#define BOARD_LCD_SPI_PORT           TUYA_SPI_NUM_2
#define BOARD_LCD_SPI_CLK            48000000
#define BOARD_LCD_SPI_CS_PIN         TUYA_GPIO_NUM_23
#define BOARD_LCD_SPI_DC_PIN         TUYA_GPIO_NUM_7
#define BOARD_LCD_SPI_RST_PIN        TUYA_GPIO_NUM_8
#endif

#define BOARD_LCD_POWER_PIN          TUYA_GPIO_NUM_MAX
#define BOARD_LCD_POWER_ACTIVE_LV    TUYA_GPIO_LEVEL_HIGH

#if defined(ENABLE_EYES_TWO_LCD) && (ENABLE_EYES_TWO_LCD == 1)

/*
SPI0驱动副屏        SCL        P14        SPI0时钟        
        CS        P15        SPI0片选        
        SDA        P16        SPI0数据        
        RST        P45        屏幕复位        可修改
        DC        P13       数据/命令选择        
        BLK        P47        屏幕背光     
*/
#define BOARD_LCD_SPI2_PORT          TUYA_SPI_NUM_0
#define BOARD_LCD_SPI2_CLK           48000000
#define BOARD_LCD_SPI2_CS_PIN        TUYA_GPIO_NUM_15
#define BOARD_LCD_SPI2_DC_PIN        TUYA_GPIO_NUM_13
#define BOARD_LCD_SPI2_RST_PIN       TUYA_GPIO_NUM_45

#endif

#elif defined (TUYA_T5AI_BOARD_EX_MODULE_096_OLED) && (TUYA_T5AI_BOARD_EX_MODULE_096_OLED ==1)
#define BOARD_LCD_BL_TYPE            TUYA_DISP_BL_TP_NONE 

#define BOARD_LCD_WIDTH              128
#define BOARD_LCD_HEIGHT             64
#define BOARD_LCD_ROTATION           TUYA_DISPLAY_ROTATION_0

#define BOARD_LCD_COLOR_INVERSE      true
#define BOARD_LCD_COM_PIN_CFG        SSD1306_COM_PIN_CFG

#define BOARD_LCD_I2C_PORT           TUYA_I2C_NUM_0
#define BOARD_LCD_I2C_SLAVER_ADDR    SSD1306_I2C_ADDR

#define BOARD_LCD_POWER_PIN          TUYA_GPIO_NUM_MAX
#endif

#if defined (ENABLE_EX_MODULE_CAMERA) && (ENABLE_EX_MODULE_CAMERA ==1)
#define BOARD_CAMERA_I2C_PORT        TUYA_I2C_NUM_0
#define BOARD_CAMERA_I2C_SCL         TUYA_GPIO_NUM_13
#define BOARD_CAMERA_I2C_SDA         TUYA_GPIO_NUM_15

#define BOARD_CAMERA_RST_PIN         TUYA_GPIO_NUM_51
#define BOARD_CAMERA_RST_ACTIVE_LV   TUYA_GPIO_LEVEL_LOW

#define BOARD_CAMERA_POWER_PIN       TUYA_GPIO_NUM_MAX

#define BOARD_CAMERA_CLK             24000000
#endif

/***********************************************************
***********************typedef define***********************
***********************************************************/


/***********************************************************
********************function declaration********************
***********************************************************/
OPERATE_RET board_register_ex_module(void);

#ifdef __cplusplus
}
#endif

#endif /* __TUYA_T5AI_EX_MODULE_H__ */
