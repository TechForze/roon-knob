# Implementation Notes

Technical documentation for the Roon Knob implementation.

## Architecture Overview

The Roon Knob firmware uses a layered architecture:

```
┌─────────────────────────────────────────┐
│         Application Layer               │
│  (roon_client.c, app_main.c)            │
└──────────────┬──────────────────────────┘
               │
┌──────────────▼──────────────────────────┐
│         Common UI Layer                  │
│  (ui.c, LVGL-based rendering)           │
└──────────────┬──────────────────────────┘
               │
┌──────────────▼──────────────────────────┐
│      Platform Abstraction Layer         │
│  (platform_*.c interfaces)              │
└──────────────┬──────────────────────────┘
               │
       ┌───────┴────────┐
       │                │
┌──────▼──────┐  ┌──────▼──────┐
│   PC Sim    │  │   ESP32-S3  │
│  (SDL2)     │  │  (ESP-IDF)  │
└─────────────┘  └─────────────┘
```

## Input System Architecture

### Overview

The input system handles user interactions through:
1. **Rotary encoder** - Volume control via hardware PCNT
2. **Buttons** - Play/pause and menu navigation via GPIO

### PC Simulator Input

**File:** `pc_sim/platform_input_pc.c`

Uses SDL keyboard events:
- Arrow keys / Mouse wheel → Volume control
- Space / Enter → Play/pause
- Z / M → Menu

### ESP32-S3 Hardware Input

**File:** `idf_app/main/platform_input_idf.c`

#### Rotary Encoder Implementation

**Hardware peripheral:** ESP32-S3 PCNT (Pulse Counter)

**Configuration:**
- Two PCNT channels for quadrature decoding (A and B)
- 4X mode: 4 pulses per detent
- Glitch filter: 1µs (1000ns)
- Polling interval: 50ms
- Counter limits: ±100

**Signal flow:**
```
Encoder rotation
    ↓
PCNT hardware decode
    ↓
Poll timer (50ms)
    ↓
Calculate delta
    ↓
Divide by 4 (detent conversion)
    ↓
ui_dispatch_input(UI_INPUT_VOL_UP/DOWN)
    ↓
roon_client_handle_input()
    ↓
HTTP POST to bridge /control endpoint
```

**Why PCNT instead of GPIO interrupts:**
- Hardware quadrature decoding (no CPU overhead)
- Built-in glitch filtering
- Automatic direction detection
- Accumulates pulses even if polling is delayed
- 16-bit counter with overflow compensation

#### Button Implementation

**Approach:** Polling-based with software debouncing

**Configuration:**
- GPIO input mode with internal pull-ups
- Active-low (button press = GPIO LOW)
- Debounce time: 50ms
- Polling interval: 50ms (shared with encoder)

**Why polling instead of interrupts:**
- Simpler debouncing logic
- No ISR overhead
- Shared timer with encoder (efficient)
- Sufficient responsiveness for UI (50ms latency)

**Signal flow:**
```
Button press
    ↓
GPIO reads LOW
    ↓
Poll timer detects edge
    ↓
Check debounce time
    ↓
ui_dispatch_input(UI_INPUT_PLAY_PAUSE/MENU)
    ↓
roon_client_handle_input()
```

### Input Event Dispatch

**Event types** (defined in `common/ui.h`):
```c
typedef enum {
    UI_INPUT_VOL_DOWN = -1,
    UI_INPUT_NONE = 0,
    UI_INPUT_VOL_UP = 1,
    UI_INPUT_PLAY_PAUSE = 2,
    UI_INPUT_MENU = 3,
} ui_input_event_t;
```

**Dispatch flow:**
```
Platform layer (keyboard/encoder/buttons)
    ↓
ui_dispatch_input(event)
    ↓
ui_input_cb_t callback
    ↓
roon_client_handle_input(event)
    ↓
[Action based on UI state]
```

**Context-aware handling:**
- When zone picker is visible:
  - VOL_UP/DOWN → Scroll zone list
  - PLAY_PAUSE → Select zone
  - MENU → Close picker
- When zone picker is hidden:
  - VOL_UP/DOWN → Volume control
  - PLAY_PAUSE → Toggle playback
  - MENU → Open zone picker

## Display System

### PC Simulator Display

**File:** `pc_sim/main_pc.c`

Uses SDL2 for rendering:
- Window size: 240×240 (matches hardware)
- LVGL driver: SDL window/renderer
- Display buffer managed by LVGL

### ESP32-S3 Hardware Display

**File:** `idf_app/main/platform_display_idf.c`

**Hardware:** SH8601 LCD controller, 360×360 resolution

**Interface:** QSPI (4-wire SPI with quad data lines)

**Configuration:**
- SPI Host: SPI2_HOST
- Clock speed: Defined by esp_lcd_sh8601 component
- Display buffer: LVGL manages 360×(360/10) = 12,960 pixels per buffer
- Double buffering for smooth rendering

**Backlight:** PWM via LEDC peripheral on GPIO 47

**Initialization sequence:**
1. Initialize SPI bus
2. Configure LCD panel IO (QSPI)
3. Load SH8601 init commands
4. Register LVGL display driver
5. Start LVGL task

## Network Stack

### mDNS Discovery

**Purpose:** Automatically discover Roon bridge on local network

**Service type:** `_roonknob._tcp`

**Files:**
- `pc_sim/platform_mdns_pc.c` - Bonjour/Avahi
- `idf_app/main/platform_mdns_idf.c` - ESP-IDF mDNS component

**Discovery flow:**
1. Query for `_roonknob._tcp` service
2. Extract IPv4 address and port
3. Construct base URL (http://IP:PORT)
4. Save to NVS storage if IP-based (not hostname)

### HTTP Client

**Files:**
- `pc_sim/components/net_client/curl_client.c` - libcurl
- `idf_app/main/platform_http_idf.c` - ESP HTTP client

**Endpoints:**
- `GET /now_playing?zone_id=X` - Poll current track/state
- `GET /zones` - List available zones
- `POST /control` - Send control commands (volume, play/pause)

**Poll interval:** 1 second (configurable in roon_client.c)

## Storage System

**Purpose:** Persist configuration across reboots

**Files:**
- `pc_sim/platform_storage_pc.c` - JSON file in home directory
- `idf_app/main/platform_storage_idf.c` - ESP NVS (Non-Volatile Storage)

**Stored configuration:**
```c
typedef struct {
    char bridge_base[128];  // e.g., "http://192.168.1.100:8088"
    char zone_id[64];       // e.g., "1234567890abcdef"
} rk_cfg_t;
```

## Threading Model

### PC Simulator
- Main thread: SDL event loop + LVGL loop
- Roon client thread: HTTP polling

### ESP32-S3
- LVGL task: UI rendering (priority: 5)
- Roon client task: HTTP polling
- Input timer: 50ms periodic timer (ISR → poll)
- IDLE tasks: FreeRTOS housekeeping

**Synchronization:**
- UI state updates: Mutex-protected
- Input events: Direct callback (LVGL-safe)
- Network state: Atomic flags

## Memory Layout (ESP32-S3)

**Flash (16 MB):**
- Bootloader: 32 KB
- Partition table: 4 KB
- App (factory): ~2 MB
- OTA_0: ~2 MB
- OTA_1: ~2 MB
- NVS: 24 KB
- SPIFFS: 8 MB (unused currently)

**RAM (8 MB PSRAM):**
- LVGL buffers: ~50 KB (display buffer)
- Network buffers: ~16 KB (HTTP responses)
- Stack: 4 KB per task
- Heap: Remainder for dynamic allocation

## Build System

### PC Simulator
```bash
cmake -B build/pc_sim
cmake --build build/pc_sim
```

**Dependencies:** SDL2, libcurl, LVGL (fetched by CMake)

### ESP32-S3
```bash
idf.py set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```

**Dependencies:** ESP-IDF v5.x, LVGL, esp_lcd_sh8601 (managed components)

## Testing Strategy

### Simulator Testing
1. Start bridge: `cd roon-extension && npm start`
2. Run simulator: `./scripts/run_pc.sh`
3. Test keyboard inputs
4. Verify HTTP traffic in bridge logs

### Hardware Testing
1. Flash firmware: `./scripts/build_flash_idf.sh /dev/cu.usbmodem101`
2. Monitor serial output: `idf.py monitor`
3. Test encoder rotation → Volume changes
4. Test button presses → Play/pause, menu
5. Verify display updates

### Integration Testing
- WiFi connection stability
- mDNS discovery reliability
- HTTP retry logic
- Zone switching
- Volume control accuracy

## Known Limitations

1. **Single zone focus:** Only one zone can be controlled at a time
2. **No track seeking:** Cannot skip forward/backward in track
3. **No playlist navigation:** Cannot skip tracks
4. **WiFi only:** No Ethernet support
5. **No OTA updates:** Requires USB flashing for firmware updates

## Future Enhancements

Tracked in Beads (.beads/issues.jsonl):
- WiFi setup screen (AP mode configuration)
- OTA update mechanism
- Long-press actions (e.g., skip track)
- Display brightness control
- Sleep mode / power saving
- Multi-zone switching UI
