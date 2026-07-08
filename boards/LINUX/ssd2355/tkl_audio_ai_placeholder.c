/**
 * @file tkl_audio_ai_placeholder.c
 * @brief Placeholder implementations of the on-device VAD / KWS (keyword
 *        spotting) audio AI hooks for the SigmaStar SSD2355 (glibc) board.
 * @version 1.0
 * @date 2026-07-08
 * @copyright Copyright (c) 2021-2026 Tuya Inc. All Rights Reserved.
 *
 * @note The real VAD/KWS implementations (platform/LINUX tkl_vad.c / tkl_kws.c)
 *       depend on the prebuilt `audio_subsys` and `MNN` libraries, which Tuya
 *       only ships for the aarch64 boards (DshanPi_A1 / Raspberry_Pi). They are
 *       not available for the 32-bit armv7 SSD2355, so this board provides weak,
 *       no-op placeholders that let the full your_chat_bot application link and
 *       run. With these placeholders the device works in push-to-talk style
 *       (no on-device wake word, VAD always reports "no speech").
 *
 *       To enable real on-device wake word / VAD on SSD2355, drop armv7 builds of
 *       libaudio_subsys / libMNN / libopus into
 *       platform/LINUX/tuyaos_adapter/src/tkl_audio/libs/ssd2355/ and enable
 *       CONFIG_ENABLE_AUDIO; the genuine tkl_vad.c / tkl_kws.c will then be used
 *       instead of these placeholders.
 */
#include "tkl_vad.h"
#include "tkl_kws.h"

/* ---------------------------------------------------------------------------
 * VAD (Voice Activity Detection) placeholders
 * --------------------------------------------------------------------------- */
/**
 * @brief Placeholder: set VAD sensitivity threshold.
 * @param[in] level requested threshold level (ignored).
 * @return OPRT_NOT_SUPPORTED always.
 */
OPERATE_RET tkl_vad_set_threshold(TKL_AUDIO_VAD_THRESHOLD_E level)
{
    (void)level;
    return OPRT_NOT_SUPPORTED;
}

/**
 * @brief Placeholder: initialize the VAD engine.
 * @param[in] config VAD configuration (ignored).
 * @return OPRT_NOT_SUPPORTED always.
 */
OPERATE_RET tkl_vad_init(TKL_VAD_CONFIG_T *config)
{
    (void)config;
    return OPRT_NOT_SUPPORTED;
}

/**
 * @brief Placeholder: feed PCM samples to the VAD engine.
 * @param[in] data PCM data (ignored).
 * @param[in] len data length in bytes (ignored).
 * @return OPRT_NOT_SUPPORTED always.
 */
OPERATE_RET tkl_vad_feed(uint8_t *data, uint32_t len)
{
    (void)data;
    (void)len;
    return OPRT_NOT_SUPPORTED;
}

/**
 * @brief Placeholder: query current VAD status.
 * @return TKL_VAD_STATUS_NONE always (never reports speech).
 */
TKL_VAD_STATUS_T tkl_vad_get_status(void)
{
    return TKL_VAD_STATUS_NONE;
}

/**
 * @brief Placeholder: start VAD detection.
 * @return OPRT_NOT_SUPPORTED always.
 */
OPERATE_RET tkl_vad_start(void)
{
    return OPRT_NOT_SUPPORTED;
}

/**
 * @brief Placeholder: stop VAD detection.
 * @return OPRT_NOT_SUPPORTED always.
 */
OPERATE_RET tkl_vad_stop(void)
{
    return OPRT_NOT_SUPPORTED;
}

/**
 * @brief Placeholder: deinitialize the VAD engine.
 * @return OPRT_NOT_SUPPORTED always.
 */
OPERATE_RET tkl_vad_deinit(void)
{
    return OPRT_NOT_SUPPORTED;
}

/* ---------------------------------------------------------------------------
 * KWS (Keyword Spotting / wake word) placeholders
 * --------------------------------------------------------------------------- */
/**
 * @brief Placeholder: initialize the keyword spotting engine.
 * @return OPRT_NOT_SUPPORTED always.
 */
OPERATE_RET tkl_kws_init(void)
{
    return OPRT_NOT_SUPPORTED;
}

/**
 * @brief Placeholder: register the wake-word callback.
 * @param[in] wakeup_cb wake-word callback (ignored).
 * @return OPRT_NOT_SUPPORTED always.
 */
OPERATE_RET tkl_kws_reg_wakeup_cb(TKL_KWS_WAKEUP_CB wakeup_cb)
{
    (void)wakeup_cb;
    return OPRT_NOT_SUPPORTED;
}

/**
 * @brief Placeholder: enable keyword spotting.
 * @return OPRT_NOT_SUPPORTED always.
 */
OPERATE_RET tkl_kws_enable(void)
{
    return OPRT_NOT_SUPPORTED;
}

/**
 * @brief Placeholder: disable keyword spotting.
 * @return OPRT_NOT_SUPPORTED always.
 */
OPERATE_RET tkl_kws_disable(void)
{
    return OPRT_NOT_SUPPORTED;
}

/**
 * @brief Placeholder: deinitialize the keyword spotting engine.
 * @return OPRT_NOT_SUPPORTED always.
 */
OPERATE_RET tkl_kws_deinit(void)
{
    return OPRT_NOT_SUPPORTED;
}
