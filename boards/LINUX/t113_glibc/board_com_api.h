/**
 * @file board_com_api.h
 * @brief Common board-level hardware registration API for the Allwinner T113
 *        (glibc) Linux board.
 * @version 1.0
 * @date 2026-06-01
 * @copyright Copyright (c) 2021-2026 Tuya Inc. All Rights Reserved.
 */
#ifndef __BOARD_COM_API_H__
#define __BOARD_COM_API_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "tuya_cloud_types.h"

/* ---------------------------------------------------------------------------
 * Function declarations
 * --------------------------------------------------------------------------- */
/**
 * @brief Register all hardware peripherals available on the T113 board.
 * @return OPRT_OK on success, error code otherwise.
 * @note Optional peripherals (ALSA audio, keyboard input) are compiled in only
 *       when the matching Kconfig options are enabled.
 */
OPERATE_RET board_register_hardware(void);

#ifdef __cplusplus
}
#endif

#endif /* __BOARD_COM_API_H__ */
