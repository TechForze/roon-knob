#include "mdns_client.h"

#include <stdio.h>
#include <esp_err.h>
#include <esp_log.h>
#include <string.h>
#include <mdns.h>

static const char *TAG = "mdns_client";
const char *SERVICE = "_roonknob._tcp";

int mdns_client_init(void) {
    esp_err_t err = mdns_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "mdns init failed: %s", esp_err_to_name(err));
        return -1;
    }
    mdns_hostname_set("roon-knob");
    mdns_instance_name_set("Roon Knob Device");
    return 0;
}

int mdns_client_query_bridge(char *out_base, size_t max_len) {
    mdns_result_t *results = NULL;
    esp_err_t err = mdns_query_ptr(SERVICE, 1000, 1, &results);
    if (err != ESP_OK || !results) {
        return -1;
    }

    if (results->service && results->service->instance_name) {
        const char *host = results->service->hostname;
        uint16_t port = results->service->port;
        snprintf(out_base, max_len, "http://%s:%u", host, port);
    }

    mdns_query_results_free(results);
    return 0;
}
