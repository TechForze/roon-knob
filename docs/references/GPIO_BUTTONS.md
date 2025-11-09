# GPIO Button Input for ESP32-S3

Reference documentation for implementing button input using ESP32-S3 GPIO with debouncing.

## Hardware Overview

Buttons are typically connected in one of two configurations:
1. **Active low**: Button connects GPIO to GND (use internal pull-up)
2. **Active high**: Button connects GPIO to VCC (use internal pull-down)

Most designs use active-low with internal pull-ups to minimize external components.

## GPIO Configuration

```c
#define BUTTON_GPIO_NUM  GPIO_NUM_X
```

For active-low buttons:
```c
gpio_config_t io_conf = {
    .pin_bit_mask = (1ULL << BUTTON_GPIO_NUM),
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_ENABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_NEGEDGE,  // Trigger on press
};
gpio_config(&io_conf);
```

## Debouncing Strategies

### 1. Software Debouncing
Simple time-based approach:
```c
#define DEBOUNCE_TIME_MS 50

static uint32_t last_press_time = 0;

void button_isr_handler(void* arg) {
    uint32_t now = esp_log_timestamp();
    if (now - last_press_time < DEBOUNCE_TIME_MS) {
        return;  // Ignore bounce
    }
    last_press_time = now;
    // Handle button press
}
```

### 2. State Machine Debouncing
More robust for multi-button setups:
```c
typedef enum {
    BUTTON_STATE_IDLE,
    BUTTON_STATE_PRESSED,
    BUTTON_STATE_RELEASED,
} button_state_t;

static button_state_t button_state = BUTTON_STATE_IDLE;
static uint32_t state_entry_time = 0;

void button_poll(void) {
    bool pressed = (gpio_get_level(BUTTON_GPIO_NUM) == 0);
    uint32_t now = esp_log_timestamp();

    switch (button_state) {
        case BUTTON_STATE_IDLE:
            if (pressed) {
                button_state = BUTTON_STATE_PRESSED;
                state_entry_time = now;
            }
            break;

        case BUTTON_STATE_PRESSED:
            if (now - state_entry_time >= DEBOUNCE_TIME_MS) {
                if (pressed) {
                    // Confirmed press
                    handle_button_press();
                    button_state = BUTTON_STATE_RELEASED;
                } else {
                    button_state = BUTTON_STATE_IDLE;
                }
            }
            break;

        case BUTTON_STATE_RELEASED:
            if (!pressed && (now - state_entry_time >= DEBOUNCE_TIME_MS)) {
                button_state = BUTTON_STATE_IDLE;
            }
            break;
    }
}
```

## Interrupt vs Polling

### Interrupt-driven (Lower Latency)
```c
static QueueHandle_t button_evt_queue = NULL;

static void IRAM_ATTR button_isr_handler(void* arg) {
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(button_evt_queue, &gpio_num, NULL);
}

void button_init(void) {
    button_evt_queue = xQueueCreate(10, sizeof(uint32_t));

    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_GPIO_NUM, button_isr_handler,
                         (void*) BUTTON_GPIO_NUM);
}

void button_task(void *arg) {
    uint32_t gpio_num;
    while (1) {
        if (xQueueReceive(button_evt_queue, &gpio_num, portMAX_DELAY)) {
            vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_TIME_MS));
            if (gpio_get_level(gpio_num) == 0) {
                // Confirmed button press
                handle_button_press();
            }
        }
    }
}
```

### Polling-driven (Simpler, Lower Power)
```c
void platform_input_poll(void) {
    static uint32_t last_poll = 0;
    uint32_t now = esp_log_timestamp();

    if (now - last_poll < 20) {  // Poll every 20ms
        return;
    }
    last_poll = now;

    button_poll();  // From state machine above
}
```

## Multi-Button Matrix

For multiple buttons, use a polling loop:
```c
typedef struct {
    gpio_num_t gpio;
    uint32_t last_press;
    bool was_pressed;
    void (*callback)(void);
} button_t;

static button_t buttons[] = {
    {.gpio = GPIO_NUM_X, .callback = handle_play_pause},
    {.gpio = GPIO_NUM_Y, .callback = handle_menu},
};

void buttons_poll(void) {
    uint32_t now = esp_log_timestamp();

    for (int i = 0; i < sizeof(buttons) / sizeof(button_t); i++) {
        bool pressed = (gpio_get_level(buttons[i].gpio) == 0);

        if (pressed && !buttons[i].was_pressed) {
            if (now - buttons[i].last_press >= DEBOUNCE_TIME_MS) {
                buttons[i].callback();
                buttons[i].last_press = now;
            }
        }
        buttons[i].was_pressed = pressed;
    }
}
```

## Integration with LVGL

For LVGL integration, buttons can:
1. **Direct event injection**: Call `lv_event_send()` on screen objects
2. **Input device**: Use LVGL's button input device type
3. **Custom events**: Dispatch through `ui_dispatch_input()` callback

Example:
```c
void handle_button_press(void) {
    ui_dispatch_input(UI_INPUT_PLAY_PAUSE);
}
```

## Common Button Types

| Action | Typical Implementation |
|--------|------------------------|
| Play/Pause | Single GPIO, active-low with pull-up |
| Menu/Select | Rotary encoder press (shaft button) |
| Reset | Long press detection (>3s) |
| Multi-function | Short press vs long press state machine |

## Power Considerations

For low-power designs:
- Use GPIO wakeup: `gpio_wakeup_enable()`
- Configure as EXT1 wakeup source for deep sleep
- Disable GPIO interrupts when not needed

## ESP Component Registry

Consider using existing button component:
- `espressif/button` from esp-iot-solution
- Provides debouncing, multi-click, long-press detection
- Well-tested and maintained

## References

- [ESP-IDF GPIO API](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-reference/peripherals/gpio.html)
- [ESP-IoT-Solution Button Component](https://docs.espressif.com/projects/esp-iot-solution/en/latest/input_device/button.html)
