#pragma once

#include "rk_cfg.h"
#include "ui.h"
#include <stdbool.h>

void roon_client_start(const rk_cfg_t *cfg);
void roon_client_handle_input(ui_input_event_t event);
void roon_client_set_network_ready(bool ready);
