#include "config_store.h"

#include <esp_err.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <nvs.h>
#include <string.h>

static const char *TAG = "config_store";
static nvs_handle_t s_handle;

int config_store_init(void) {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs init failed: %s", esp_err_to_name(err));
        return -1;
    }

    err = nvs_open("rook", NVS_READWRITE, &s_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs open failed: %s", esp_err_to_name(err));
        return -1;
    }
    return 0;
}

static int config_string_get(const char *key, char *out, size_t max_len) {
    size_t required = max_len;
    esp_err_t err = nvs_get_str(s_handle, key, out, &required);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        out[0] = '\0';
        return 1;
    }
    if (err != ESP_OK) {
        return -1;
    }
    return 0;
}

static int config_string_set(const char *key, const char *value) {
    esp_err_t err = nvs_set_str(s_handle, key, value);
    if (err != ESP_OK) {
        return -1;
    }
    return nvs_commit(s_handle) == ESP_OK ? 0 : -1;
}

int config_store_get_bridge_base(char *out, size_t max_len) {
    return config_string_get("bridge_base", out, max_len);
}

int config_store_set_bridge_base(const char *base) {
    return config_string_set("bridge_base", base);
}

int config_store_get_zone_id(char *out, size_t max_len) {
    return config_string_get("zone_id", out, max_len);
}

int config_store_set_zone_id(const char *zone) {
    return config_string_set("zone_id", zone);
}
