#include "app.h"
#include "platform/platform_input.h"
#include "platform/platform_time.h"
#include "ui.h"
#include "ui_network.h"
#include "wifi_manager.h"

#include "lvgl.h"

#include <esp_err.h>
#include <nvs_flash.h>

void rk_net_evt_cb(rk_net_evt_t evt, const char *ip_opt) {
    ui_network_on_event(evt, ip_opt);
}

void app_main(void) {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        esp_err_t erase_err = nvs_flash_erase();
        if (erase_err != ESP_OK) {
            // ignore
        }
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    wifi_mgr_start();
    lv_init();
    ui_init();
    ui_network_register_menu();
    platform_input_init();
    app_entry();

    while (true) {
        ui_loop_iter();
        platform_sleep_ms(5);
    }
}
