#include "tkl_gpio.h"
#include "tkl_i2c.h"
#include "tal_log.h"
#include "tal_thread.h"
#include "tal_semaphore.h"
#include "app_gesture.h"

#define GESTURE_INT_PIN       TUYA_GPIO_NUM_22
#define GESTURE_I2C_ADDR      0x73 // PAJ7620 I2C 地址
#define GESTURE_REG_CHIP_ID_L 0x00
#define GESTURE_REG_CHIP_ID_H 0x01

// 手势类型定义
typedef enum {
    APP_GES_NONE = 0,
    APP_GES_UP,
    APP_GES_DOWN,
    APP_GES_LEFT,
    APP_GES_RIGHT,
    APP_GES_FORWARD,
    APP_GES_BACKWARD,
    APP_GES_CLOCKWISE,
    APP_GES_COUNTERCLOCKWISE,
    APP_GES_WAVE
} APP_GESTURE_TYPE_E;

const uint8_t init_reg_arr[][2] = {
    // BANK 0
    {0xEF,0x00}, {0x37,0x07}, {0x38,0x17}, {0x39,0x06}, {0x42,0x01}, 
    {0x46,0x2D}, {0x47,0x0F}, {0x48,0x3C}, {0x49,0x00}, {0x4A,0x1E}, 
    {0x4C,0x20}, {0x51,0x10}, {0x5E,0x10}, {0x60,0x27}, {0x80,0x42}, 
    {0x81,0x44}, {0x82,0x04}, {0x8B,0x01}, {0x90,0x06}, {0x95,0x0A}, 
    {0x96,0x0C}, {0x97,0x05}, {0x9A,0x14}, {0x9C,0x3F}, {0xA5,0x19}, 
    {0xCC,0x19}, {0xCD,0x0B}, {0xCE,0x13}, {0xCF,0x64}, {0xD0,0x21}, 
    // BANK 1
    {0xEF,0x01}, {0x02,0x0F}, {0x03,0x10}, {0x04,0x02}, {0x25,0x01},
    {0x27,0x39}, {0x28,0x7F}, {0x29,0x08}, {0x3E,0xFF}, {0x5E,0x3D}, 
    {0x65,0x96}, {0x67,0x97}, {0x69,0xCD}, {0x6A,0x01}, {0x6D,0x2C}, 
    {0x6E,0x01}, {0x72,0x01}, {0x73,0x35}, {0x77,0x01}, {0xEF,0x00},
};

STATIC THREAD_HANDLE s_irq_thread_handle = NULL;  // 中断线程句柄
STATIC SEM_HANDLE s_irq_semaphore = NULL; // 中断信号量句柄

STATIC VOID __irq_cb(void *arg)
{
    if (s_irq_semaphore) {
        tal_semaphore_post(s_irq_semaphore);
    }

    return;
}

STATIC VOID __gesture_irq_thread(VOID *arg)
{
    while (1) {
        PR_NOTICE("semaphore wait");
        tal_semaphore_wait(s_irq_semaphore, SEM_WAIT_FOREVER);
        PR_NOTICE("I2C read");
    }
}

static uint8_t __gesture_i2c_read_uint8(uint8_t reg)
{
    uint8_t value = 0;
    tkl_i2c_master_send(TUYA_I2C_NUM_0, GESTURE_I2C_ADDR, &reg, 1, TRUE);
    tkl_i2c_master_receive(TUYA_I2C_NUM_0, GESTURE_I2C_ADDR, &value, 1, FALSE);
    PR_DEBUG("read reg: %02x, value: %02x", reg, value);
    return value;
}

static int __gesture_i2c_write_uint8(uint8_t reg, uint8_t value)
{
    OPERATE_RET rt = OPRT_OK;
    uint8_t data[2] = {reg, value};
    rt = tkl_i2c_master_send(TUYA_I2C_NUM_0, GESTURE_I2C_ADDR, data, 2, FALSE);
    PR_DEBUG("write reg: %02x, value: %02x", GESTURE_I2C_ADDR, reg, value);
    return rt;
}

OPERATE_RET app_gesture_init(VOID)
{
    OPERATE_RET rt = OPRT_OK;
    int i = 0;
    uint16_t chip_id = 0;
    TUYA_GPIO_BASE_CFG_T gpio_cfg = {
        .mode = TUYA_GPIO_PULLUP,
        .direct = TUYA_GPIO_INPUT,
        .level = TUYA_GPIO_LEVEL_HIGH
    };
    TUYA_GPIO_IRQ_T irq_cfg = {
        .mode = TUYA_GPIO_IRQ_LOW,
        .cb = __irq_cb,
        .arg = NULL,
    };
    TUYA_IIC_BASE_CFG_T i2c_cfg = {
        .role = TUYA_IIC_MODE_MASTER,
        .speed = TUYA_IIC_BUS_SPEED_100K,
        .addr_width = TUYA_IIC_ADDRESS_7BIT,
    };
    THREAD_CFG_T thrd_param = {0};
    thrd_param.thrdname = "gesture_irq";
    thrd_param.priority = THREAD_PRIO_1;
    thrd_param.stackDepth = 1024;

    TUYA_CALL_ERR_GOTO(tkl_gpio_init(GESTURE_INT_PIN, &gpio_cfg), ERR_EXIT);
    TUYA_CALL_ERR_GOTO(tal_semaphore_create_init(&s_irq_semaphore, 0, 1), ERR_EXIT);
    TUYA_CALL_ERR_GOTO(tkl_gpio_irq_init(GESTURE_INT_PIN, &irq_cfg), ERR_EXIT);
    TUYA_CALL_ERR_GOTO(tkl_gpio_irq_enable(GESTURE_INT_PIN), ERR_EXIT);

    TUYA_CALL_ERR_GOTO(tkl_i2c_init(TUYA_I2C_NUM_0, &i2c_cfg), ERR_EXIT);

    // 读取 Chip ID
    chip_id = __gesture_i2c_read_uint8(GESTURE_REG_CHIP_ID_H);
    chip_id = (chip_id << 8) | __gesture_i2c_read_uint8(GESTURE_REG_CHIP_ID_L);
    if (chip_id != 0x7620) { // 检查芯片ID
        PR_ERR("Gesture sensor not found, chip_id: 0x%04X", chip_id);
        return OPRT_COM_ERROR;
    }

    for (i = 0; i < CNTSOF(init_reg_arr); i++) {
        tkl_i2c_master_send(TUYA_I2C_NUM_0, GESTURE_I2C_ADDR, init_reg_arr[i], 2, FALSE);
    }

    rt = tal_thread_create_and_start(&s_irq_thread_handle, NULL, NULL, __gesture_irq_thread, NULL, &thrd_param);
    if (rt != OPRT_OK) {
        PR_ERR("tal_thread_create_and_start failed, rt: %d", rt);
        goto ERR_EXIT;
    }

    PR_INFO("Gesture sensor initialized successfully");

    return OPRT_OK;

ERR_EXIT:
    if (s_irq_semaphore) {
        tal_semaphore_release(s_irq_semaphore);
        s_irq_semaphore = NULL;
    }
    return rt;
}