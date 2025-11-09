#include "storage.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STORAGE_MAX_ENTRIES 32
#define STORAGE_LINE_SIZE 256

static char storage_path[PATH_MAX];

struct entry {
    char key[64];
    char value[192];
};

static int read_entries(struct entry *entries, size_t *count) {
    FILE *f = fopen(storage_path, "r");
    if (!f) {
        return -1;
    }
    size_t idx = 0;
    char line[STORAGE_LINE_SIZE];
    while (fgets(line, sizeof(line), f) && idx < STORAGE_MAX_ENTRIES) {
        char *newline = strchr(line, '\n');
        if (newline) *newline = '\0';
        char *eq = strchr(line, '=');
        if (!eq) continue;
        *eq = '\0';
        strncpy(entries[idx].key, line, sizeof(entries[idx].key) - 1);
        entries[idx].key[sizeof(entries[idx].key) - 1] = '\0';
        strncpy(entries[idx].value, eq + 1, sizeof(entries[idx].value) - 1);
        entries[idx].value[sizeof(entries[idx].value) - 1] = '\0';
        idx++;
    }
    fclose(f);
    *count = idx;
    return 0;
}

static int write_entries(struct entry *entries, size_t count) {
    FILE *f = fopen(storage_path, "w");
    if (!f) return -1;
    for (size_t i = 0; i < count; ++i) {
        fprintf(f, "%s=%s\n", entries[i].key, entries[i].value);
    }
    fclose(f);
    return 0;
}

int storage_init(void) {
    const char *home = getenv("HOME");
    if (!home) return -1;
    if (snprintf(storage_path, sizeof(storage_path), "%s/.roon_knob_storage", home) >= (int)sizeof(storage_path)) {
        return -1;
    }
    return 0;
}

int storage_get(const char *key, char *out, size_t len) {
    if (!key || !out || !len || storage_path[0] == '\0') return -1;
    struct entry entries[STORAGE_MAX_ENTRIES];
    size_t count = 0;
    if (read_entries(entries, &count) != 0) return -1;
    for (size_t i = 0; i < count; ++i) {
        if (strcmp(entries[i].key, key) == 0) {
            strncpy(out, entries[i].value, len - 1);
            out[len - 1] = '\0';
            return 0;
        }
    }
    return -1;
}

int storage_set(const char *key, const char *value) {
    if (!key || !value || storage_path[0] == '\0') return -1;
    struct entry entries[STORAGE_MAX_ENTRIES];
    size_t count = 0;
    read_entries(entries, &count);
    for (size_t i = 0; i < count; ++i) {
        if (strcmp(entries[i].key, key) == 0) {
            strncpy(entries[i].value, value, sizeof(entries[i].value) - 1);
            entries[i].value[sizeof(entries[i].value) - 1] = '\0';
            return write_entries(entries, count);
        }
    }
    if (count < STORAGE_MAX_ENTRIES) {
        strncpy(entries[count].key, key, sizeof(entries[count].key) - 1);
        entries[count].key[sizeof(entries[count].key) - 1] = '\0';
        strncpy(entries[count].value, value, sizeof(entries[count].value) - 1);
        entries[count].value[sizeof(entries[count].value) - 1] = '\0';
        count++;
        return write_entries(entries, count);
    }
    return -1;
}

void storage_close(void) {
}
