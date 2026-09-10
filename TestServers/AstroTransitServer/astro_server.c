#define _GNU_SOURCE
#include <arpa/inet.h>
#include <ctype.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include "constellations.h"
#include "transit.h"

#define MAX_REQUEST 65536

static void send_response(int fd, int status, const char *status_text, const char *body, size_t body_len) {
    char header[256];
    int hlen = snprintf(header, sizeof(header),
                         "HTTP/1.1 %d %s\r\n"
                         "Content-Type: application/json\r\n"
                         "Content-Length: %zu\r\n"
                         "Connection: close\r\n"
                         "\r\n",
                         status, status_text, body_len);
    write(fd, header, (size_t)hlen);
    if (body_len > 0) write(fd, body, body_len);
}

static void send_error(int fd, int status, const char *status_text, const char *message) {
    char body[256];
    int blen = snprintf(body, sizeof(body), "{\"error\":\"%s\"}", message);
    send_response(fd, status, status_text, body, (size_t)blen);
}

static int hex_val(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static void url_decode(const char *src, size_t src_len, char *out, size_t out_len) {
    size_t j = 0;
    for (size_t i = 0; i < src_len && j + 1 < out_len; i++) {
        if (src[i] == '+') {
            out[j++] = ' ';
        } else if (src[i] == '%' && i + 2 < src_len) {
            int hi = hex_val(src[i + 1]), lo = hex_val(src[i + 2]);
            if (hi >= 0 && lo >= 0) {
                out[j++] = (char)((hi << 4) | lo);
                i += 2;
            } else {
                out[j++] = src[i];
            }
        } else {
            out[j++] = src[i];
        }
    }
    out[j] = '\0';
}

static int query_param(const char *query, const char *key, char *out, size_t out_len) {
    size_t key_len = strlen(key);
    const char *p = query;
    while (p && *p) {
        if (strncmp(p, key, key_len) == 0 && p[key_len] == '=') {
            const char *value = p + key_len + 1;
            const char *end = strchr(value, '&');
            size_t len = end ? (size_t)(end - value) : strlen(value);
            url_decode(value, len, out, out_len);
            return 0;
        }
        p = strchr(p, '&');
        if (p) p++;
    }
    return -1;
}

static void handle_transit(int fd, const char *query) {
    char lat_str[32], lng_str[32], name[64];
    if (query_param(query, "lat", lat_str, sizeof(lat_str)) != 0 ||
        query_param(query, "lng", lng_str, sizeof(lng_str)) != 0 ||
        query_param(query, "constellation", name, sizeof(name)) != 0) {
        send_error(fd, 400, "Bad Request", "lat, lng, and constellation are all required");
        return;
    }

    char *end;
    double lat = strtod(lat_str, &end);
    if (*end != '\0' || lat < -90.0 || lat > 90.0) {
        send_error(fd, 400, "Bad Request", "lat must be a number between -90 and 90");
        return;
    }
    double lng = strtod(lng_str, &end);
    if (*end != '\0' || lng < -180.0 || lng > 180.0) {
        send_error(fd, 400, "Bad Request", "lng must be a number between -180 and 180");
        return;
    }

    const constellation_t *c = constellation_find(name);
    if (!c) {
        char msg[128];
        snprintf(msg, sizeof(msg), "unknown constellation '%s'", name);
        send_error(fd, 404, "Not Found", msg);
        return;
    }

    transit_result_t result;
    compute_transit(lat, lng, c, time(NULL), &result);

    struct tm transit_tm;
    gmtime_r(&result.transit_utc, &transit_tm);
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%SZ", &transit_tm);

    char body[512];
    int blen = snprintf(body, sizeof(body),
        "{\"constellation\":\"%s\",\"latitude\":%.4f,\"longitude\":%.4f,"
        "\"transitTimeUtc\":\"%s\",\"maxAltitudeDeg\":%.2f,\"visibility\":\"%s\"}",
        c->name, lat, lng, timestamp, result.max_altitude_deg, result.visibility);

    send_response(fd, 200, "OK", body, (size_t)blen);
}

static char *read_http_request(int fd) {
    char *buf = malloc(MAX_REQUEST);
    size_t len = 0;
    for (;;) {
        if (len >= MAX_REQUEST - 1) { free(buf); return NULL; }
        ssize_t n = read(fd, buf + len, MAX_REQUEST - 1 - len);
        if (n <= 0) { free(buf); return NULL; }
        len += (size_t)n;
        buf[len] = '\0';
        if (strstr(buf, "\r\n\r\n")) break;
    }
    return buf;
}

int main(int argc, char **argv) {
    int port = 8070;
    if (argc > 1) port = atoi(argv[1]);

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) { perror("socket"); return 1; }

    int yes = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons((uint16_t)port);

    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) != 0) { perror("bind"); return 1; }
    if (listen(listen_fd, 16) != 0) { perror("listen"); return 1; }

    printf("faux astronomical transit service listening on http://127.0.0.1:%d/transit\n", port);
    fflush(stdout);

    for (;;) {
        int fd = accept(listen_fd, NULL, NULL);
        if (fd < 0) continue;

        char *req = read_http_request(fd);
        if (req) {
            char method[8] = "", path[256] = "";
            sscanf(req, "%7s %255s", method, path);

            if (strcmp(method, "GET") == 0 && strncmp(path, "/transit", 8) == 0) {
                const char *query = strchr(path, '?');
                handle_transit(fd, query ? query + 1 : "");
            } else {
                send_error(fd, 404, "Not Found", "not found");
            }
            free(req);
        }
        close(fd);
    }
}
