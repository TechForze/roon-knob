#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ui.h"
#include "components/net_client/curl_client.h"

#define POLL_INTERVAL_SECONDS 1
#define MAX_LINE 128

static char bridge_base[256] = "http://127.0.0.1:8088";
static char zone_id[64] = "office";
static bool net_ok = false;
static int net_volume_step = 2;

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

static void fetch_now_playing(struct now_playing *state) {
    char url[512];
    snprintf(url, sizeof(url), "%s/now_playing?zone_id=%s", bridge_base, zone_id);
    char *resp = NULL;
    size_t resp_len = 0;
    int ret = http_get(url, &resp, &resp_len);
    if (ret != 0 || !resp) {
        net_ok = false;
        http_free(resp);
        return;
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
}

static void send_control_json(const char *json) {
    char url[512];
    snprintf(url, sizeof(url), "%s/control", bridge_base);
    char *resp = NULL;
    size_t resp_len = 0;
    if (http_post_json(url, json, &resp, &resp_len) != 0) {
        fprintf(stderr, "[pc_sim] control failed %s\n", json);
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

int main(int argc, char **argv) {
    const char *env_base = getenv("ROON_BRIDGE_BASE");
    if (env_base && env_base[0]) {
        snprintf(bridge_base, sizeof(bridge_base), "%s", env_base);
    }
    const char *env_zone = getenv("ZONE_ID");
    if (env_zone && env_zone[0]) {
        snprintf(zone_id, sizeof(zone_id), "%s", env_zone);
    }

    ui_init();
    ui_set_input_handler(handle_input);
    struct now_playing state; 
    default_now_playing(&state);
    ui_update(&state.line1[0], &state.line2[0], state.is_playing, state.volume);
    ui_set_status(net_ok);

    while (true) {
        fetch_now_playing(&state);
        ui_update(state.line1, state.line2, state.is_playing, state.volume);
        ui_set_status(net_ok);
        sleep(POLL_INTERVAL_SECONDS);
    }

    return 0;
}
