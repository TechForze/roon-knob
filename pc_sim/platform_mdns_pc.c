#include "platform/platform_mdns.h"

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

void platform_mdns_init(const char *hostname) {
    (void)hostname;
}

bool platform_mdns_discover_base_url(char *out, size_t len) {
    if (!out || len == 0) {
        return false;
    }
    const char *env = getenv("ROON_BRIDGE_BASE");
    if (!env || !env[0]) {
        return false;
    }
    strncpy(out, env, len - 1);
    out[len - 1] = '\0';
    return true;
}
