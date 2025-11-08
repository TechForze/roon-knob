#include <stdio.h>
#include <stdbool.h>

#include "ui.h"

void ui_init(void) {
    puts("[pc_sim] UI initialized (placeholder)");
}

void ui_update(const char *line1, const char *line2, bool playing, int volume) {
    printf("[pc_sim] Now playing: %s / %s %s (vol %d)\n", line1, line2, playing ? "[playing]" : "[paused]", volume);
}

void ui_set_status(bool online) {
    printf("[pc_sim] Network status: %s\n", online ? "ok" : "error");
}
