/**
 * @file board_com_api.c
 * @brief Common board-level hardware registration for the SigmaStar SSD2355
 *        (glibc) Linux board.
 * @version 1.0
 * @date 2026-07-08
 * @copyright Copyright (c) 2021-2026 Tuya Inc. All Rights Reserved.
 */
#include "board_com_api.h"
#include "tal_api.h"

#if defined(ENABLE_AUDIO_ALSA) && (ENABLE_AUDIO_ALSA == 1)
#include "tdd_audio_alsa.h"
#endif

#if defined(ENABLE_KEYBOARD_INPUT) && (ENABLE_KEYBOARD_INPUT == 1)
#include "tdd_button_keyboard.h"
#endif

#include "tuya_cloud_types.h"
#include <string.h>

/* ---------------------------------------------------------------------------
 * Function implementations
 * --------------------------------------------------------------------------- */
/**
 * @brief Register the ALSA audio device when ALSA support is enabled.
 * @return OPRT_OK on success, error code otherwise.
 */
STATIC OPERATE_RET __board_register_audio(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;

#if defined(ENABLE_AUDIO_ALSA) && (ENABLE_AUDIO_ALSA == 1)
#if defined(AUDIO_CODEC_NAME)
    TDD_AUDIO_ALSA_CFG_T alsa_cfg = {0};

#if defined(ALSA_DEVICE_CAPTURE)
    strncpy(alsa_cfg.capture_device, ALSA_DEVICE_CAPTURE, sizeof(alsa_cfg.capture_device) - 1);
#else
    strncpy(alsa_cfg.capture_device, "plughw:0,0", sizeof(alsa_cfg.capture_device) - 1);
#endif

#if defined(ALSA_DEVICE_PLAYBACK)
    strncpy(alsa_cfg.playback_device, ALSA_DEVICE_PLAYBACK, sizeof(alsa_cfg.playback_device) - 1);
#else
    strncpy(alsa_cfg.playback_device, "default", sizeof(alsa_cfg.playback_device) - 1);
#endif

    alsa_cfg.sample_rate = TDD_ALSA_SAMPLE_16000;
    alsa_cfg.data_bits = TDD_ALSA_DATABITS_16;
    alsa_cfg.channels = TDD_ALSA_CHANNEL_MONO;
    alsa_cfg.spk_sample_rate = TDD_ALSA_SAMPLE_16000;
    alsa_cfg.buffer_frames = 1024;
    alsa_cfg.period_frames = 256;

#if defined(ENABLE_AUDIO_AEC) && (ENABLE_AUDIO_AEC == 1)
    alsa_cfg.aec_enable = 1;
#else
    alsa_cfg.aec_enable = 0;
#endif

    rt = tdd_audio_alsa_register(AUDIO_CODEC_NAME, alsa_cfg);
    if (OPRT_OK != rt) {
        PR_WARN("Failed to register ALSA audio driver: %d", rt);
        return rt;
    }
    PR_INFO("ALSA audio device registered: %s", AUDIO_CODEC_NAME);
#else
    PR_WARN("AUDIO_CODEC_NAME not defined, skipping audio registration");
#endif
#else
    PR_DEBUG("ALSA audio support not enabled");
#endif

    return rt;
}

/**
 * @brief Register the keyboard button when keyboard input is enabled.
 * @return OPRT_OK on success, error code otherwise.
 */
STATIC OPERATE_RET __board_register_button(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;

#if defined(ENABLE_KEYBOARD_INPUT) && (ENABLE_KEYBOARD_INPUT == 1)
#if defined(BUTTON_NAME)
    BUTTON_CFG_T btn_cfg = {0};
    btn_cfg.mode = BUTTON_TIMER_SCAN_MODE;
    rt = tdd_keyboard_button_register(BUTTON_NAME, &btn_cfg);
    if (OPRT_OK != rt) {
        PR_ERR("Failed to register keyboard button: %d", rt);
        return rt;
    }
    PR_INFO("Keyboard button registered: %s", BUTTON_NAME);
#endif
#else
    PR_DEBUG("Keyboard input not enabled");
#endif

    return rt;
}

/**
 * @brief Register all hardware peripherals available on the SSD2355 board.
 * @return OPRT_OK on success, error code otherwise.
 * @note Optional peripherals (ALSA audio, keyboard input) are compiled in only
 *       when the matching Kconfig options are enabled.
 */
OPERATE_RET board_register_hardware(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;

    PR_INFO("Registering SigmaStar SSD2355 board hardware...");

    rt = __board_register_audio();
    if (OPRT_OK != rt) {
        PR_WARN("Audio registration skipped/failed: %d", rt);
    }

    rt = __board_register_button();
    if (OPRT_OK != rt) {
        PR_WARN("Button registration skipped/failed: %d", rt);
    }

    PR_INFO("SSD2355 board hardware registration completed");

    return OPRT_OK;
}
