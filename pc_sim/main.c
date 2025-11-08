#include <pthread.h>
#include <ctype.h>
#include <strings.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#include "ui.h"
#include "components/net_client/curl_client.h"

#define POLL_INTERVAL_SECONDS 1
#define MAX_LINE 128

static char bridge_base[256] = "http://127.0.0.1:8088";
static char zone_id[64] = "";
static bool net_ok = false;
static int net_volume_step = 2;
static volatile bool run_threads = true;
static char zone_label[64] = "Loading zone";
static bool zone_resolved = false;

struct now_playing {
    char line1[MAX_LINE];
    char line2[MAX_LINE];
    bool is_playing;
    int volume;
    int volume_step;
};

static void default_now_playing(struct now_playing *state) {
    snprintf(state->line1, sizeof(state->line1), "%s", "Waiting for data");
    state->line2[0] = '\0';
    state->is_playing = false;
    state->volume = 0;
    state->volume_step = net_volume_step;
}

static void curl_string_copy(const char *data, const char *key, char *out, size_t len) {
    const char *pos = strstr(data, key);
    if (!pos) return;
    const char *start = strchr(pos, '"');
    if (!start) return;
    start++;
    const char *end = strchr(start, '"');
    if (!end) return;
    size_t copy_len = end - start;
    if (copy_len >= len) copy_len = len - 1;
    memcpy(out, start, copy_len);
    out[copy_len] = '\0';
}

static bool refresh_zone_label(void);
static void log_msg(const char *fmt, ...);

static bool fetch_now_playing(struct now_playing *state) {
    if (!zone_resolved) {
        refresh_zone_label();
        if (!zone_resolved) {
            log_msg("cannot resolve zone; skipping poll");
            return false;
        }
    }
    if (zone_id[0] == '\0') {
        refresh_zone_label();
        if (zone_id[0] == '\0') {
            log_msg("zone_id still empty after refresh");
            return false;
        }
    }
    char url[512];
    snprintf(url, sizeof(url), "%s/now_playing?zone_id=%s", bridge_base, zone_id);
    char *resp = NULL;
    size_t resp_len = 0;
    int ret = http_get(url, &resp, &resp_len);
    if (ret != 0 || !resp) {
        net_ok = false;
        log_msg("now_playing request failed (ret=%d)", ret);
        http_free(resp);
        return false;
    }
    if (resp_len == 0 || strstr(resp, "\"error\"")) {
        net_ok = false;
        zone_resolved = false;
        log_msg("now_playing returned error or empty payload: %.*s", (int)resp_len, resp ? resp : "");
        http_free(resp);
        return false;
    }

    curl_string_copy(resp, "\"line1\"", state->line1, sizeof(state->line1));
    curl_string_copy(resp, "\"line2\"", state->line2, sizeof(state->line2));
    if (strstr(resp, "\"is_playing\":true")) {
        state->is_playing = true;
    } else {
        state->is_playing = false;
    }
    const char *vol_key = strstr(resp, "\"volume\"");
    if (vol_key) {
        const char *colon = strchr(vol_key, ':');
        if (colon) {
            state->volume = atoi(colon + 1);
        }
    }

    state->volume_step = net_volume_step;
    const char *step_key = strstr(resp, "\"volume_step\"");
    if (step_key) {
        const char *colon = strchr(step_key, ':');
        if (colon) {
            int parsed = atoi(colon + 1);
            if (parsed > 0) {
                state->volume_step = parsed;
            }
        }
    }
    net_volume_step = state->volume_step;

    net_ok = true;
    http_free(resp);
    return true;
}

static void send_control_json(const char *json) {
    char url[512];
    snprintf(url, sizeof(url), "%s/control", bridge_base);
    char *resp = NULL;
    size_t resp_len = 0;
    if (http_post_json(url, json, &resp, &resp_len) != 0) {
        log_msg("control failed payload=%s", json);
    }
    http_free(resp);
}

static void handle_input(ui_input_event_t ev) {
    switch (ev) {
        case UI_INPUT_VOL_DOWN: {
            int step = net_volume_step > 0 ? net_volume_step : 2;
            char body[256];
            snprintf(body, sizeof(body), "{\"zone_id\":\"%s\",\"action\":\"vol_rel\",\"value\":%d}", zone_id, -step);
            send_control_json(body);
            break;
        }
        case UI_INPUT_VOL_UP: {
            int step = net_volume_step > 0 ? net_volume_step : 2;
            char body[256];
            snprintf(body, sizeof(body), "{\"zone_id\":\"%s\",\"action\":\"vol_rel\",\"value\":%d}", zone_id, step);
            send_control_json(body);
            break;
        }
        case UI_INPUT_PLAY_PAUSE: {
            char body[256];
            snprintf(body, sizeof(body), "{\"zone_id\":\"%s\",\"action\":\"play_pause\"}", zone_id);
            send_control_json(body);
            break;
        }
        default:
            break;
    }
}

static void *poll_thread(void *arg) {
    struct now_playing state;
    default_now_playing(&state);
    while (run_threads) {
        bool ok = fetch_now_playing(&state);
        if (ok) {
            ui_update(state.line1, state.line2, state.is_playing, state.volume);
            ui_set_status(true);
            ui_set_message("Connected");
        } else {
            ui_set_status(false);
            ui_set_message("Waiting for data...");
        }
        sleep(POLL_INTERVAL_SECONDS);
    }
    return NULL;
}

static bool refresh_zone_label(void) {
    char url[512];
    snprintf(url, sizeof(url), "%s/zones", bridge_base);
    char *resp = NULL;
    size_t resp_len = 0;
    if (http_get(url, &resp, &resp_len) != 0 || !resp) {
        ui_set_zone_name(zone_label);
        http_free(resp);
        zone_resolved = false;
        return false;
    }
    const char *cursor = resp;
    char first_id[64] = "";
    char first_name[64] = "";
    bool found = false;
    bool want_any = zone_id[0] == '\0';
    while ((cursor = strstr(cursor, "\"zone_id\""))) {
        const char *id_start = strchr(cursor, '"');
        if (!id_start) break;
        id_start = strchr(id_start + 1, '"');
        if (!id_start) break;
        id_start++;
        const char *id_end = strchr(id_start, '"');
        if (!id_end) break;
        size_t id_len = id_end - id_start;
        if (id_len >= sizeof(first_id)) id_len = sizeof(first_id) - 1;
        char current_id[64];
        memcpy(current_id, id_start, id_len);
        current_id[id_len] = '\0';

        const char *name_key = strstr(id_end, "\"zone_name\"");
        if (!name_key) break;
        const char *name_start = strchr(name_key, '"');
        if (!name_start) break;
        name_start = strchr(name_start + 1, '"');
        if (!name_start) break;
        name_start++;
        const char *name_end = strchr(name_start, '"');
        if (!name_end) break;
        size_t name_len = name_end - name_start;
        if (name_len >= sizeof(first_name)) name_len = sizeof(first_name) - 1;
        char current_name[64];
        memcpy(current_name, name_start, name_len);
        current_name[name_len] = '\0';

        if (first_id[0] == '\0') {
            snprintf(first_id, sizeof(first_id), "%s", current_id);
            snprintf(first_name, sizeof(first_name), "%s", current_name);
        }

        bool id_match = zone_id[0] && strcmp(current_id, zone_id) == 0;
        bool name_match = zone_id[0] && strcasecmp(current_name, zone_id) == 0;
        if (want_any || id_match || name_match) {
            snprintf(zone_id, sizeof(zone_id), "%s", current_id);
            snprintf(zone_label, sizeof(zone_label), "%s", current_name);
            ui_set_zone_name(zone_label);
            found = true;
            zone_resolved = true;
            break;
        }
        cursor = name_end + 1;
    }

    if (!found && first_id[0]) {
        snprintf(zone_id, sizeof(zone_id), "%s", first_id);
        snprintf(zone_label, sizeof(zone_label), "%s", first_name);
        ui_set_zone_name(zone_label);
        zone_resolved = true;
        log_msg("zone fallback -> id=%s name=%s", zone_id, zone_label);
    } else if (!found) {
        ui_set_zone_name(zone_label);
        zone_resolved = false;
        log_msg("zones fetch did not resolve any zone");
    }
    http_free(resp);
    return zone_resolved;
}
int main(int argc, char **argv) {
    const char *env_base = getenv("ROON_BRIDGE_BASE");
    if (env_base && env_base[0]) {
        snprintf(bridge_base, sizeof(bridge_base), "%s", env_base);
    }
    const char *env_zone = getenv("ZONE_ID");
    if (env_zone && env_zone[0]) {
        snprintf(zone_id, sizeof(zone_id), "%s", env_zone);
        snprintf(zone_label, sizeof(zone_label), "%s", env_zone);
    } else {
        zone_id[0] = '\0';
        snprintf(zone_label, sizeof(zone_label), "%s", "Loading zone");
    }
    ui_init();
    ui_set_zone_name(zone_label);
    ui_set_input_handler(handle_input);
    refresh_zone_label();

    pthread_t net_thread;
    pthread_create(&net_thread, NULL, poll_thread, NULL);

    while (true) {
        ui_loop_iter();
        usleep(5000);
    }
}

static void log_msg(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "[pc_sim] ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
}
