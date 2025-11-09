# Hardware Pin Configuration

GPIO pin assignments for the Roon Knob ESP32-S3 hardware.

## Display Pins (ST77916 LCD via QSPI)

| Function | GPIO | Notes |
|----------|------|-------|
| LCD_CS | 14 | SPI Chip Select |
| LCD_PCLK | 13 | SPI Clock |
| LCD_DATA0 | 15 | QSPI Data 0 |
| LCD_DATA1 | 16 | QSPI Data 1 |
| LCD_DATA2 | 17 | QSPI Data 2 |
| LCD_DATA3 | 18 | QSPI Data 3 |
| LCD_RST | 21 | Reset line |
| BK_LIGHT | 47 | Backlight PWM |

Display configuration: 360×360 round LCD with ST77916 controller using 4-wire QSPI interface.

## Input Pins (Rotary Encoder + Touch)

### Rotary Encoder

| Function | GPIO | Notes |
|----------|------|-------|
| ENCODER_A (ECA) | 8 | Quadrature channel A |
| ENCODER_B (ECB) | 7 | Quadrature channel B |

**Implementation:**
- Software quadrature decoding with polling (3ms interval)
- Software debouncing (2 ticks)
- Internal pull-ups enabled
- Generates UI_INPUT_VOL_UP/VOL_DOWN events

**Note:** This device has NO physical buttons. The encoder shaft is NOT pressable. All touch interactions are handled via the CST816D touch controller.

### Touch Controller (CST816D)

| Function | GPIO | I2C Details |
|----------|------|-------------|
| SDA | 11 | I2C_NUM_0, pull-up enabled |
| SCL | 12 | I2C_NUM_0, pull-up enabled |
| I2C Address | 0x15 | 7-bit address |
| I2C Speed | 300 kHz | |

**Implementation:**
- CST816D capacitive touch controller
- 12-bit coordinate resolution (0-4095 raw, mapped to 0-359 for display)
- Integrated with LVGL as LV_INDEV_TYPE_POINTER
- Touch events automatically handled by LVGL input system

**Touch to I2C Mapping:**
```
CST816D SDA → GPIO 11 (I2C_NUM_0)
CST816D SCL → GPIO 12 (I2C_NUM_0)
I2C Speed: 300 kHz
```

## GPIO Availability Summary

| GPIO Range | Status | Usage |
|------------|--------|-------|
| 7-8 | **Used** | Rotary encoder (ECB, ECA) |
| 11-12 | **Used** | Touch controller I2C (SDA, SCL) |
| 13-18 | **Used** | Display data/control |
| 21 | **Used** | Display reset |
| 47 | **Used** | Backlight PWM |
| 0-6, 9-10, 19-20, 22-46, 48 | **Available** | Expansion |

## Reserved/Strapping Pins

Be cautious when using these ESP32-S3 pins:
- **GPIO0**: Boot mode selection (keep floating or pulled high)
- **GPIO45**: VDD_SPI voltage selection
- **GPIO46**: Boot mode selection

## Power Considerations

- All GPIO pins operate at 3.3V logic levels
- Maximum current per GPIO: 40mA (20mA recommended)
- Total GPIO current should not exceed 200mA

## Alternative Pin Assignments

If the default pins conflict with your hardware, you can modify the pin assignments in:
- `idf_app/main/platform_display_idf.c` for display pins
- `idf_app/main/platform_input_idf.c` for input pins

**Important:** Ensure any alternative pins do not conflict with:
- Strapping pins (0, 45, 46)
- Flash/PSRAM pins (26-32 on some modules)
- USB pins (19-20 if using native USB)

## Testing Pin Assignments

To verify your pin configuration:
1. Check continuity between board connector and ESP32-S3 module pins
2. Verify no conflicts with strapping pins
3. Test encoder rotation direction (swap A/B if reversed)
4. Verify touch events register in serial monitor logs
5. Test I2C communication with CST816D (should see initialization logs)
