#include "encoder_input.h"
#include <stdio.h>

void encoder_input_init(void) {
    printf("[idf] Encoder input placeholder initialized\n");
}

int encoder_get_delta(void) {
    return 0;
}

bool encoder_pressed(void) {
    return false;
}
