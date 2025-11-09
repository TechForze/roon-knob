#pragma once

#include <stdbool.h>
#include <stddef.h>

void platform_mdns_init(const char *hostname);
bool platform_mdns_discover_base_url(char *out, size_t len);
