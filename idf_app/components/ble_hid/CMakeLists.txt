# BLE HID Component
# Source: Espressif ESP-IDF examples (CC0/Unlicense)
# Reference implementation from BlueKnob project (Apache 2.0)

if(CONFIG_ROON_KNOB_BLE_HID_ENABLED)
    idf_component_register(
        SRCS
            "esp_hidd_prf_api.c"
            "hid_dev.c"
            "hid_device_le_prf.c"
        INCLUDE_DIRS
            "."
        REQUIRES
            bt
            esp_event
    )
else()
    # Register empty component when BLE is disabled
    idf_component_register()
endif()
