#include "app.h"
#include "platform/platform_input.h"
#include "platform/platform_time.h"
#include "ui.h"

#include "lvgl.h"
#include "src/drivers/sdl/lv_sdl_mouse.h"
#include "src/drivers/sdl/lv_sdl_window.h"

#define SCREEN_SIZE 240

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
