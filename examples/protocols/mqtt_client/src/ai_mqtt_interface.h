/**
 * @file ai_mqtt_interface.h
 * @brief AI MQTT interface for sending commands and receiving responses
 *
 * This file provides interfaces for:
 * - Sending data to AI_CMD topic
 * - Receiving messages from AI_RET topic via callback
 *
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 */

#ifndef AI_MQTT_INTERFACE_H
#define AI_MQTT_INTERFACE_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* AI message callback function type */
typedef void (*ai_ret_message_cb_t)(const uint8_t *payload, size_t length, void *userdata);

/**
 * @brief Send data to AI_CMD topic
 *
 * @param data Data to send
 * @param length Data length
 * @param qos Quality of Service level (0, 1, or 2)
 * @return uint16_t Message ID if successful, 0 if failed
 */
uint16_t ai_cmd_send(const uint8_t *data, size_t length, uint8_t qos);

/**
 * @brief Register callback for AI_RET topic messages
 *
 * @param callback Callback function to handle AI_RET messages
 * @param userdata User data to pass to callback
 * @return int 0 on success, -1 on failure
 */
int ai_ret_register_callback(ai_ret_message_cb_t callback, void *userdata);

/**
 * @brief Unregister AI_RET callback
 */
void ai_ret_unregister_callback(void);

#ifdef __cplusplus
}
#endif

#endif /* AI_MQTT_INTERFACE_H */

