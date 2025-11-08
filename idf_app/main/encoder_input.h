#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void encoder_input_init(void);
int encoder_get_delta(void);
bool encoder_pressed(void);

#ifdef __cplusplus
}
#endif
