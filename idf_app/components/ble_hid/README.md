# BLE HID Component

BLE (Bluetooth Low Energy) HID (Human Interface Device) implementation for ESP32.

## Source

This code is from Espressif's ESP-IDF examples and is licensed under CC0/Unlicense (public domain).

Reference implementation from [BlueKnob project](https://github.com/joshuacant/BlueKnob) (Apache 2.0).

## Usage

Enable BLE HID mode in menuconfig:

```
idf.py menuconfig
→ Roon Knob Configuration
  → Enable BLE HID mode
```

Or build with the BLE defaults:

```bash
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.defaults.ble" build
```

## API

```c
#include "esp_hidd_prf_api.h"
#include "hid_dev.h"

// Initialize BLE HID profile
esp_hidd_profile_init();

// Send media key (volume, play/pause, etc.)
esp_hidd_send_consumer_value(conn_id, HID_CONSUMER_VOLUME_UP, true);
esp_hidd_send_consumer_value(conn_id, HID_CONSUMER_VOLUME_UP, false);

// Send keyboard key
uint8_t key = HID_KEY_RETURN;
esp_hidd_send_keyboard_value(conn_id, 0, &key, 1);

// Send mouse movement
esp_hidd_send_mouse_value(conn_id, 0, delta_x, delta_y);
```

## Consumer Key Codes

From `hid_dev.h`:

- `HID_CONSUMER_VOLUME_UP` / `HID_CONSUMER_VOLUME_DOWN`
- `HID_CONSUMER_PLAY` / `HID_CONSUMER_PAUSE`
- `HID_CONSUMER_SCAN_NEXT_TRK` / `HID_CONSUMER_SCAN_PREV_TRK`
- `HID_CONSUMER_FAST_FORWARD` / `HID_CONSUMER_REWIND`
- `HID_CONSUMER_POWER`
