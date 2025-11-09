#pragma once

#include "rk_cfg.h"
#include "ui.h"

void roon_client_start(const rk_cfg_t *cfg);
void roon_client_handle_input(ui_input_event_t event);
