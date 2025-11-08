#include "ui.h"
#include <stdio.h>

void ui_init(void) {
    printf("[idf] UI placeholder init\n");
}

void ui_update(const char *title, const char *subtitle, bool playing, int volume) {
    printf("[idf] UI update: %s / %s %s (vol %d)\n", title, subtitle, playing ? "[playing]" : "[paused]", volume);
}

void ui_set_status(bool online) {
    printf("[idf] Network status: %s\n", online ? "online" : "offline");
}
