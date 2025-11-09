#pragma once

#include <stddef.h>

int storage_init(void);
int storage_get(const char *key, char *out, size_t len);
int storage_set(const char *key, const char *value);
void storage_close(void);
