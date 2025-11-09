#include "platform/platform_http.h"
#include "components/net_client/curl_client.h"

int platform_http_get(const char *url, char **out, size_t *out_len) {
    return http_get(url, out, out_len);
}

int platform_http_post_json(const char *url, const char *json, char **out, size_t *out_len) {
    return http_post_json(url, json, out, out_len);
}

void platform_http_free(char *p) {
    http_free(p);
}
