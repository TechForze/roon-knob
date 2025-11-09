ESP32‑S3 1.8" Knob Display – Board Overview

Summary
- High‑integration dual‑MCU design (ESP32‑S3 + ESP32), up to 4 cores total @ 240 MHz.
- 2.4 GHz Wi‑Fi (802.11 b/g/n) + BT 5 (LE/Classic) with onboard antenna.
- On‑board 16 MB Flash + 8 MB PSRAM (in addition to ESP32‑S3 internal 512 KB SRAM, 384 KB ROM).
- 1.8" round IPS LCD, 360 × 360, 262 K color, ~600 cd/m², 1200:1 contrast.
- LCD over QSPI, Driver IC: ST77916.
- Capacitive touch (self‑capacitance) via I2C, Touch IC: CST816, with INT and RST.
- Peripherals: audio decoder, microphone, rotary knob (quadrature encoder + push), vibration motor, TF card slot, battery charge management.
- CNC metal case.

What Matters For Firmware
- Display: 360×360 ST77916 over QSPI. Ensure pinout and clock meet the controller’s timing; backlight on dedicated PWM.
- Touch: CST816 on I2C with INT (active‑low) and RST. See CST816D notes for init and event flow.
- Input: Rotary encoder A/B + push for primary UI controls.
- Audio: Decoder + mic are available (not yet used by the Roon UI flow).
- Storage: 16 MB Flash + 8 MB PSRAM allow LVGL buffers and networking without tight constraints.

Quick Specs Table
- MCUs: ESP32‑S3 + ESP32, up to 4 cores @ 240 MHz
- Wireless: 2.4 GHz Wi‑Fi + BT 5 (LE/Classic)
- Memory: 16 MB Flash, 8 MB PSRAM, 512 KB SRAM (internal)
- Display: 1.8" IPS, 360×360, 262 K colors, ~600 cd/m², 1200:1
- LCD IF: QSPI, Driver IC ST77916
- Touch: Capacitive, I2C (CST816), INT + RST
- Peripherals: audio decoder, mic, knob encoder, vibration motor, TF slot, charge mgmt

Related Docs
- Touch controller integration: docs/reference/hardware/cst816d.md
- Pinout summary: docs/references/HARDWARE_PINS.md

