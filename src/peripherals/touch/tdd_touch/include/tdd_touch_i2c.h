/**
 * @file tdd_touch_i2c.h
 * @version 0.1
 * @date 2025-06-09
 */

#ifndef __TDD_TOUCH_I2C_H__
#define __TDD_TOUCH_I2C_H__

#include "tuya_cloud_types.h"

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
    TUYA_I2C_NUM_E  port;
    TUYA_PIN_NAME_E scl_pin;
    TUYA_PIN_NAME_E sda_pin;
}TDD_TOUCH_I2C_CFG_T;

/***********************************************************
********************function declaration********************
***********************************************************/
void tdd_touch_i2c_pinmux_config(TDD_TOUCH_I2C_CFG_T *cfg);

OPERATE_RET tdd_touch_i2c_port_read(TUYA_I2C_NUM_E port, uint16_t dev_addr, \
                                    uint16_t reg_addr, uint8_t reg_addr_len,\
                                    uint8_t *data, uint8_t data_len);

OPERATE_RET tdd_touch_i2c_port_write(TUYA_I2C_NUM_E port, uint16_t dev_addr,\
                                     uint16_t reg_addr, uint8_t reg_addr_len,\
                                     uint8_t *data, uint8_t data_len);                                  

#ifdef __cplusplus
}
#endif

#endif /* __TDD_TOUCH_I2C_H__ */
