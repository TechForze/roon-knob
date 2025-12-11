/**
 * @file ble_hid_client.h
 * @brief BLE HID client for Bluetooth media control mode
 *
 * This module implements Bluetooth Low Energy HID (Human Interface Device)
 * support for controlling media on phones, laptops, TVs, etc.
 */

#ifndef BLE_HID_CLIENT_H
#define BLE_HID_CLIENT_H

#include "ui.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief BLE connection state
 */
typedef enum {
    BLE_HID_STATE_DISABLED,     /**< BLE stack not initialized */
    BLE_HID_STATE_ADVERTISING,  /**< Advertising, waiting for connection */
    BLE_HID_STATE_CONNECTED     /**< Connected to a device */
} ble_hid_state_t;

/**
 * @brief BLE state change callback type
 * @param state New state
 * @param device_name Name of connected device (NULL if not connected)
 */
typedef void (*ble_hid_state_cb_t)(ble_hid_state_t state, const char *device_name);

/**
 * @brief Initialize and start BLE HID
 *
 * Initializes Bluetooth controller and starts advertising.
 * Only available if CONFIG_ROON_KNOB_BLE_HID_ENABLED is set.
 *
 * @return true if started successfully, false otherwise
 */
bool ble_hid_client_start(void);

/**
 * @brief Stop BLE HID and deinitialize Bluetooth
 */
void ble_hid_client_stop(void);

/**
 * @brief Handle input event in BLE mode
 *
 * Maps ui_input_event_t to BLE HID consumer commands:
 * - VOL_UP/DOWN -> HID_CONSUMER_VOLUME_UP/DOWN
 * - PLAY_PAUSE -> HID_CONSUMER_PLAY
 * - NEXT_TRACK -> HID_CONSUMER_SCAN_NEXT_TRK
 * - PREV_TRACK -> HID_CONSUMER_SCAN_PREV_TRK
 *
 * @param event Input event to handle
 */
void ble_hid_client_handle_input(ui_input_event_t event);

/**
 * @brief Get current BLE connection state
 * @return Current state (disabled, advertising, or connected)
 */
ble_hid_state_t ble_hid_client_get_state(void);

/**
 * @brief Register callback for state changes
 * @param callback Function to call on state change
 */
void ble_hid_client_set_state_callback(ble_hid_state_cb_t callback);

/**
 * @brief Get name of connected device
 * @return Device name or NULL if not connected
 */
const char *ble_hid_client_get_device_name(void);

/**
 * @brief Check if BLE HID support is compiled in
 * @return true if BLE HID is available
 */
bool ble_hid_client_available(void);

#ifdef __cplusplus
}
#endif

#endif /* BLE_HID_CLIENT_H */
