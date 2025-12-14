#include "platform/platform_storage.h"

#include <esp_err.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <nvs.h>
#include <stdlib.h>
#include <string.h>

#include "sdkconfig.h"

static const char *TAG = "platform_storage";
static const char *NAMESPACE = "rk_cfg";
static const char *KEY = "cfg";

static void ensure_version(rk_cfg_t *cfg) {
    if (!cfg) {
        return;
    }
    if (cfg->cfg_ver == 0) {
        cfg->cfg_ver = RK_CFG_CURRENT_VER;
    }
}

static esp_err_t open_ns(nvs_handle_t *handle, nvs_open_mode_t mode) {
    return nvs_open(NAMESPACE, mode, handle);
}

bool platform_storage_load(rk_cfg_t *out) {
    if (!out) {
        return false;
    }
    esp_err_t err;
    nvs_handle_t handle;
    err = open_ns(&handle, NVS_READONLY);
    if (err != ESP_OK) {
        if (err != ESP_ERR_NVS_NOT_FOUND) {
            ESP_LOGW(TAG, "nvs open failed: %s", esp_err_to_name(err));
        }
        return false;
    }
    size_t len = sizeof(*out);
    err = nvs_get_blob(handle, KEY, out, &len);
    nvs_close(handle);
    if (err != ESP_OK || len != sizeof(*out)) {
        if (err != ESP_ERR_NVS_NOT_FOUND) {
            ESP_LOGW(TAG, "nvs read failed: %s", esp_err_to_name(err));
        }
        memset(out, 0, sizeof(*out));
        return false;
    }
    ensure_version(out);
    // Log all fields for debugging
    ESP_LOGI(TAG, "Loaded config: ssid='%s' bridge='%s' zone='%s' ver=%d",
             out->ssid[0] ? out->ssid : "(empty)",
             out->bridge_base[0] ? out->bridge_base : "(empty)",
             out->zone_id[0] ? out->zone_id : "(empty)",
             out->cfg_ver);

    return true;
}

bool platform_storage_save(const rk_cfg_t *in) {
    if (!in) {
        return false;
    }
    rk_cfg_t copy = *in;
    ensure_version(&copy);

    ESP_LOGI(TAG, "Saving config: ssid='%s' bridge='%s' zone='%s' ver=%d",
             copy.ssid[0] ? copy.ssid : "(empty)",
             copy.bridge_base[0] ? copy.bridge_base : "(empty)",
             copy.zone_id[0] ? copy.zone_id : "(empty)",
             copy.cfg_ver);

    esp_err_t err;
    nvs_handle_t handle;
    err = open_ns(&handle, NVS_READWRITE);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs open rw failed: %s", esp_err_to_name(err));
        return false;
    }
    err = nvs_set_blob(handle, KEY, &copy, sizeof(copy));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_set_blob failed: %s", esp_err_to_name(err));
        nvs_close(handle);
        return false;
    }
    ESP_LOGI(TAG, "nvs_set_blob OK, committing...");

    err = nvs_commit(handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_commit failed: %s", esp_err_to_name(err));
        nvs_close(handle);
        return false;
    }
    ESP_LOGI(TAG, "nvs_commit OK");
    nvs_close(handle);

    // Verify by reading back
    rk_cfg_t verify = {0};
    if (!platform_storage_load(&verify)) {
        ESP_LOGE(TAG, "VERIFY FAILED: Could not read back saved config!");
        return false;
    }

    if (strcmp(verify.ssid, copy.ssid) != 0) {
        ESP_LOGE(TAG, "VERIFY FAILED: SSID mismatch! saved='%s' read='%s'",
                 copy.ssid, verify.ssid);
        return false;
    }

    ESP_LOGI(TAG, "VERIFY OK: Config saved and verified successfully");
    return true;
}

void platform_storage_defaults(rk_cfg_t *out) {
    if (!out) {
        return;
    }
    memset(out, 0, sizeof(*out));
    // Leave bridge_base empty - mDNS discovery is the primary method
    // wifi_manager will fill SSID/pass from Kconfig defaults
    // zone_id is left empty - user will select from available zones
    ESP_LOGI(TAG, "Applied defaults (bridge will be discovered via mDNS)");
    out->cfg_ver = RK_CFG_CURRENT_VER;
}

void platform_storage_reset_wifi_only(rk_cfg_t *cfg) {
    if (!cfg) {
        return;
    }
    cfg->ssid[0] = '\0';
    cfg->pass[0] = '\0';
    platform_storage_save(cfg);
}
