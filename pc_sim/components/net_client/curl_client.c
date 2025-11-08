#include "curl_client.h"

#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct curl_buffer {
    char *data;
    size_t len;
};

static size_t curl_write(void *ptr, size_t size, size_t nmemb, void *userdata) {
    size_t total = size * nmemb;
    struct curl_buffer *buf = userdata;
    char *next = realloc(buf->data, buf->len + total + 1);
    if (!next) {
        return 0;
    }
    buf->data = next;
    memcpy(buf->data + buf->len, ptr, total);
    buf->len += total;
    buf->data[buf->len] = '\0';
    return total;
}

static int curl_perform(const char *url, const char *body, const char *content_type, char **out, size_t *out_len) {
    CURL *handle = curl_easy_init();
    if (!handle) {
        return -1;
    }
    struct curl_buffer buf = {0};
    curl_easy_setopt(handle, CURLOPT_URL, url);
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, curl_write);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, &buf);
    curl_easy_setopt(handle, CURLOPT_TIMEOUT_MS, 2500L);
    struct curl_slist *headers = NULL;
    if (body) {
        if (content_type) {
            char header[128];
            snprintf(header, sizeof(header), "Content-Type: %s", content_type);
            headers = curl_slist_append(headers, header);
        }
        curl_easy_setopt(handle, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(handle, CURLOPT_POSTFIELDS, body);
        curl_easy_setopt(handle, CURLOPT_POSTFIELDSIZE, (long)strlen(body));
    }
    CURLcode code = curl_easy_perform(handle);
    if (code != CURLE_OK) {
    curl_easy_cleanup(handle);
    curl_slist_free_all(headers);
        free(buf.data);
        return -1;
    }
    *out = buf.data;
    if (out_len) {
        *out_len = buf.len;
    }
    curl_easy_cleanup(handle);
    return 0;
}

int http_get(const char *url, char **out, size_t *out_len) {
    return curl_perform(url, NULL, NULL, out, out_len);
}

int http_post_json(const char *url, const char *json, char **out, size_t *out_len) {
    return curl_perform(url, json, "application/json", out, out_len);
}

void http_free(char *p) {
    free(p);
}
