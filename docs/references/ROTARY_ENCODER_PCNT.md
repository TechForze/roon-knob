# Rotary Encoder with ESP32-S3 PCNT

Reference documentation for implementing rotary encoder input using ESP32-S3's Pulse Counter (PCNT) peripheral.

## Hardware Overview

The ESP32-S3 has **2 PCNT units** (vs 8 on the original ESP32), each capable of tracking quadrature encoder signals.

Common rotary encoders like the EC11 output two-phase quadrature signals (A and B channels) that can be decoded to determine:
- Rotation direction (clockwise/counter-clockwise)
- Rotation speed (pulse frequency)
- Absolute position (accumulated count)

## GPIO Configuration

For a typical EC11 encoder:
```c
#define ENCODER_GPIO_A  0   // Channel A signal
#define ENCODER_GPIO_B  2   // Channel B signal
// Common/ground connects to GND
```

Both GPIOs should have internal pull-ups enabled (handled by PCNT driver).

## PCNT Setup Pattern

Based on ESP-IDF examples (`examples/peripherals/pcnt/rotary_encoder`):

### 1. Unit Configuration
```c
pcnt_unit_config_t unit_config = {
    .high_limit = 100,
    .low_limit = -100,
    .flags.accum_count = true,  // Enable 16-bit accumulation
};
pcnt_unit_handle_t pcnt_unit = NULL;
ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &pcnt_unit));
```

### 2. Glitch Filter
```c
pcnt_glitch_filter_config_t filter_config = {
    .max_glitch_ns = 1000,  // 1us glitch filter
};
ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(pcnt_unit, &filter_config));
```

### 3. Channel Setup (Two Channels for Quadrature)

**Channel A:**
```c
pcnt_chan_config_t chan_a_config = {
    .edge_gpio_num = ENCODER_GPIO_A,
    .level_gpio_num = ENCODER_GPIO_B,
};
pcnt_channel_handle_t pcnt_chan_a = NULL;
ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit, &chan_a_config, &pcnt_chan_a));

ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_a,
    PCNT_CHANNEL_EDGE_ACTION_DECREASE,
    PCNT_CHANNEL_EDGE_ACTION_INCREASE));
ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_a,
    PCNT_CHANNEL_LEVEL_ACTION_KEEP,
    PCNT_CHANNEL_LEVEL_ACTION_INVERSE));
```

**Channel B:**
```c
pcnt_chan_config_t chan_b_config = {
    .edge_gpio_num = ENCODER_GPIO_B,
    .level_gpio_num = ENCODER_GPIO_A,
};
pcnt_channel_handle_t pcnt_chan_b = NULL;
ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit, &chan_b_config, &pcnt_chan_b));

ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_b,
    PCNT_CHANNEL_EDGE_ACTION_INCREASE,
    PCNT_CHANNEL_EDGE_ACTION_DECREASE));
ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_b,
    PCNT_CHANNEL_LEVEL_ACTION_KEEP,
    PCNT_CHANNEL_LEVEL_ACTION_INVERSE));
```

### 4. Enable and Start
```c
ESP_ERROR_CHECK(pcnt_unit_enable(pcnt_unit));
ESP_ERROR_CHECK(pcnt_unit_clear_count(pcnt_unit));
ESP_ERROR_CHECK(pcnt_unit_start(pcnt_unit));
```

## Quadrature Decoding

The above configuration enables **4X mode**:
- Each complete detent (click) produces 4 pulses
- Clockwise rotation: count increases by 4
- Counter-clockwise rotation: count decreases by 4

To get actual detent steps, divide the count by 4.

## Reading Position

**Polling method:**
```c
int pulse_count = 0;
ESP_ERROR_CHECK(pcnt_unit_get_count(pcnt_unit, &pulse_count));
int detents = pulse_count / 4;
```

**Event-driven method:**
```c
// Register watch points for threshold detection
pcnt_event_callbacks_t cbs = {
    .on_reach = example_pcnt_on_reach,
};
QueueHandle_t queue = xQueueCreate(10, sizeof(int));
ESP_ERROR_CHECK(pcnt_unit_register_event_callbacks(pcnt_unit, &cbs, queue));

ESP_ERROR_CHECK(pcnt_unit_add_watch_point(pcnt_unit, EXAMPLE_PCNT_HIGH_LIMIT));
ESP_ERROR_CHECK(pcnt_unit_add_watch_point(pcnt_unit, EXAMPLE_PCNT_LOW_LIMIT));
```

## Integration with LVGL

For LVGL rotary encoder input (`lv_indev_t` with `LV_INDEV_TYPE_ENCODER`):

1. Create encoder input device driver
2. Poll PCNT in `read_cb` callback
3. Track delta since last read
4. Return encoder diff and pressed state

## Performance Considerations

- **Glitch filter**: 1000ns is suitable for EC11 encoders
- **Polling rate**: 10-50ms is adequate for UI responsiveness
- **CPU overhead**: Minimal - hardware peripheral handles decoding
- **Speed limit**: Suitable for rotary knobs < 30 pulses/sec

## Alternative: Software PCNT

For chips without PCNT (ESP32-C2, C3), use `espressif/knob` component from esp-iot-solution:
- Software-based quadrature decoding
- Uses GPIO interrupts
- Higher CPU overhead
- Suitable for low-speed encoders only

## References

- [ESP-IDF PCNT Driver API](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-reference/peripherals/pcnt.html)
- [ESP-IDF Rotary Encoder Example](https://github.com/espressif/esp-idf/tree/master/examples/peripherals/pcnt/rotary_encoder)
- [esp-iot-solution Knob Component](https://docs.espressif.com/projects/esp-iot-solution/en/latest/input_device/knob.html)
