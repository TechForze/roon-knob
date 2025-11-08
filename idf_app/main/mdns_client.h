#pragma once

#ifdef __cplusplus
extern "C" {
#endif

int mdns_client_init(void);
int mdns_client_query_bridge(char *out_base, size_t max_len);

#ifdef __cplusplus
}
#endif
