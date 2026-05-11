/**
 * @file ui_ai_chat.h
 * @brief Bottom single-line transient user/assistant text (no prefix, no frame)
 * @version 1.0
 * @date 2026-05-11
 * @copyright Copyright (c) 2026 Tuya Inc. All Rights Reserved.
 */

#ifndef __UI_AI_CHAT_H__
#define __UI_AI_CHAT_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create bottom label (call inside LVGL init with disp lock held)
 * @return 0 on success, -1 on failure
 */
int __ui_ai_chat_init(void);

/**
 * @brief Set the user line and clear the assistant buffer for a new exchange
 * @param[in] string UTF-8 user text, null-terminated
 * @return none
 */
void __ui_ai_chat_set_user_msg(char *string);

/**
 * @brief Set the assistant line from a complete non-streaming message
 * @param[in] string UTF-8 assistant text, null-terminated
 * @return none
 */
void __ui_ai_chat_set_ai_msg(char *string);

/**
 * @brief Start a streamed assistant reply (clears the answer buffer)
 * @return none
 */
void __ui_ai_chat_ai_stream_start(void);

/**
 * @brief Append one UTF-8 chunk to the streamed assistant reply
 * @param[in] string UTF-8 chunk, null-terminated
 * @return none
 */
void __ui_ai_chat_ai_stream_data(char *string);

/**
 * @brief Finish a streamed assistant reply
 * @return none
 */
void __ui_ai_chat_ai_stream_end(void);

#ifdef __cplusplus
}
#endif

#endif /* __UI_AI_CHAT_H__ */
