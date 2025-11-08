#pragma once

#ifdef __cplusplus
extern "C" {
#endif

int config_store_init(void);
int config_store_get_bridge_base(char *out, size_t max_len);
int config_store_set_bridge_base(const char *base);
int config_store_get_zone_id(char *out, size_t max_len);
int config_store_set_zone_id(const char *zone);

#ifdef __cplusplus
}
#endif
