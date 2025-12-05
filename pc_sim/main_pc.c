#include "app.h"
#include "platform/platform_input.h"
#include "platform/platform_time.h"
#include "ui.h"

#include "lvgl.h"
#include "src/drivers/sdl/lv_sdl_mouse.h"
#include "src/drivers/sdl/lv_sdl_window.h"

#include <stdio.h>

#define SCREEN_SIZE 240

// Settings UI stubs for PC simulator
void ui_show_settings(void) {
    printf("[PC] Settings: Long-press detected (settings not implemented in simulator)\n");
}

void ui_hide_settings(void) {
}

bool ui_is_settings_visible(void) {
    return false;
}

// Display sleep stub - PC simulator never sleeps
bool platform_display_is_sleeping(void) {
    return false;
}

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    lv_init();
    lv_display_t *disp = lv_sdl_window_create(SCREEN_SIZE, SCREEN_SIZE);
    lv_display_set_default(disp);
    lv_sdl_mouse_create();

    ui_init();
    platform_input_init();
    app_entry();

    while (true) {
        ui_loop_iter();
        platform_sleep_ms(5);
    }

    return 0;
}
