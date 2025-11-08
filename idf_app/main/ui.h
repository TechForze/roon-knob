#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void ui_init(void);
void ui_update(const char *title, const char *subtitle, bool playing, int volume);
void ui_set_status(bool online);

#ifdef __cplusplus
}
#endif
