#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void ui_init(void);
void ui_update(const char *line1, const char *line2, bool playing, int volume);
void ui_set_status(bool online);

#ifdef __cplusplus
}
#endif
