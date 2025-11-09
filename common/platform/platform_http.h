#pragma once

#include <stddef.h>

int platform_http_get(const char *url, char **out, size_t *out_len);
int platform_http_post_json(const char *url, const char *json, char **out, size_t *out_len);
void platform_http_free(char *p);
