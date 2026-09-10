#define _GNU_SOURCE
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#define LISTEN_BACKLOG 16
#define MAX_REQUEST 65536

static const char *CONDITIONS[] = {
    "Sunny", "Cloudy", "Rainy", "Windy", "Snowy", "Partly Cloudy", "Stormy",
};
#define NUM_CONDITIONS (sizeof(CONDITIONS) / sizeof(CONDITIONS[0]))

typedef struct {
    char city[128];
    double temperature_f;
    const char *conditions;
    int humidity_percent;
    double wind_mph;
} weather_data_t;

static unsigned long fnv1a(const char *s) {
    unsigned long h = 2166136261UL;
    for (; *s; s++) {
        h ^= (unsigned char)*s;
        h *= 16777619UL;
    }
    return h;
}

static void faux_weather_for(const char *city, weather_data_t *out) {
    unsigned long h = fnv1a(city);
    snprintf(out->city, sizeof(out->city), "%s", city);
    out->temperature_f = 30 + (double)(h % 71);
    out->conditions = CONDITIONS[(h >> 8) % NUM_CONDITIONS];
    out->humidity_percent = 20 + (int)((h >> 16) % 71);
    out->wind_mph = (double)((h >> 24) % 300) / 10.0;
}

static int extract_element(const char *xml, const char *tag, char *out, size_t out_len) {
    char open[64];
    snprintf(open, sizeof(open), "<%s", tag);
    const char *p = strstr(xml, open);
    if (!p) return -1;
    p = strchr(p, '>');
    if (!p) return -1;
    p++;

    char close[64];
    snprintf(close, sizeof(close), "</%s>", tag);
    const char *end = strstr(p, close);
    if (!end) return -1;

    size_t len = (size_t)(end - p);
    if (len >= out_len) len = out_len - 1;
    memcpy(out, p, len);
    out[len] = '\0';
    return 0;
}

static const char *WSDL_PATH = "../weather.wsdl";

static char *read_wsdl_file(size_t *out_len) {
    FILE *f = fopen(WSDL_PATH, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (size < 0) { fclose(f); return NULL; }
    char *buf = malloc((size_t)size + 1);
    if (!buf) { fclose(f); return NULL; }
    size_t n = fread(buf, 1, (size_t)size, f);
    fclose(f);
    buf[n] = '\0';
    *out_len = n;
    return buf;
}

static void send_response(int fd, int status, const char *status_text, const char *content_type,
                           const char *body, size_t body_len) {
    char header[512];
    int hlen = snprintf(header, sizeof(header),
                         "HTTP/1.1 %d %s\r\n"
                         "Content-Type: %s\r\n"
                         "Content-Length: %zu\r\n"
                         "Connection: close\r\n"
                         "\r\n",
                         status, status_text, content_type, body_len);
    write(fd, header, (size_t)hlen);
    if (body_len > 0) write(fd, body, body_len);
}

static void send_soap_fault(int fd, const char *fault_string) {
    char body[512];
    int blen = snprintf(body, sizeof(body),
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"
        "<soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">\r\n"
        "  <soap:Body>\r\n"
        "    <soap:Fault>\r\n"
        "      <faultcode>soap:Client</faultcode>\r\n"
        "      <faultstring>%s</faultstring>\r\n"
        "    </soap:Fault>\r\n"
        "  </soap:Body>\r\n"
        "</soap:Envelope>\r\n",
        fault_string);
    send_response(fd, 500, "Internal Server Error", "text/xml; charset=utf-8", body, (size_t)blen);
}

static void handle_get_weather(int fd, const char *soap_body) {
    char city[128];
    if (extract_element(soap_body, "city", city, sizeof(city)) != 0 || city[0] == '\0') {
        send_soap_fault(fd, "city is required");
        return;
    }

    weather_data_t w;
    faux_weather_for(city, &w);

    char body[1024];
    int blen = snprintf(body, sizeof(body),
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"
        "<soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">\r\n"
        "  <soap:Body>\r\n"
        "    <GetWeatherResponse xmlns=\"http://example.com/weather\">\r\n"
        "      <GetWeatherResult>\r\n"
        "        <City>%s</City>\r\n"
        "        <TemperatureF>%.1f</TemperatureF>\r\n"
        "        <Conditions>%s</Conditions>\r\n"
        "        <HumidityPercent>%d</HumidityPercent>\r\n"
        "        <WindMph>%.1f</WindMph>\r\n"
        "      </GetWeatherResult>\r\n"
        "    </GetWeatherResponse>\r\n"
        "  </soap:Body>\r\n"
        "</soap:Envelope>\r\n",
        w.city, w.temperature_f, w.conditions, w.humidity_percent, w.wind_mph);

    send_response(fd, 200, "OK", "text/xml; charset=utf-8", body, (size_t)blen);
}

static void handle_wsdl(int fd) {
    size_t len;
    char *wsdl = read_wsdl_file(&len);
    if (!wsdl) {
        const char *msg = "WSDL not found";
        send_response(fd, 500, "Internal Server Error", "text/plain", msg, strlen(msg));
        return;
    }
    send_response(fd, 200, "OK", "text/xml; charset=utf-8", wsdl, len);
    free(wsdl);
}

static void handle_not_found(int fd) {
    const char *msg = "Not Found";
    send_response(fd, 404, "Not Found", "text/plain", msg, strlen(msg));
}

static char *read_http_request(int fd, size_t *out_len) {
    char *buf = malloc(MAX_REQUEST);
    size_t len = 0;
    ssize_t n;

    const char *header_end;
    for (;;) {
        if (len >= MAX_REQUEST - 1) { free(buf); return NULL; }
        n = read(fd, buf + len, MAX_REQUEST - 1 - len);
        if (n <= 0) { free(buf); return NULL; }
        len += (size_t)n;
        buf[len] = '\0';
        header_end = strstr(buf, "\r\n\r\n");
        if (header_end) break;
    }

    size_t headers_len = (size_t)(header_end - buf) + 4;
    long content_length = 0;
    const char *cl = strcasestr(buf, "Content-Length:");
    if (cl && cl < header_end) content_length = strtol(cl + strlen("Content-Length:"), NULL, 10);

    size_t body_have = len - headers_len;
    while (content_length > 0 && body_have < (size_t)content_length) {
        if (len >= MAX_REQUEST - 1) break;
        n = read(fd, buf + len, MAX_REQUEST - 1 - len);
        if (n <= 0) break;
        len += (size_t)n;
        body_have += (size_t)n;
        buf[len] = '\0';
    }

    *out_len = len;
    return buf;
}

int main(int argc, char **argv) {
    int port = 8080;
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
    if (listen(listen_fd, LISTEN_BACKLOG) != 0) { perror("listen"); return 1; }

    printf("faux weather SOAP service listening on http://127.0.0.1:%d/weather\n", port);
    fflush(stdout);

    for (;;) {
        int fd = accept(listen_fd, NULL, NULL);
        if (fd < 0) continue;

        size_t len;
        char *req = read_http_request(fd, &len);
        if (req) {
            if (strncmp(req, "POST", 4) == 0) {
                char *body_start = strstr(req, "\r\n\r\n");
                handle_get_weather(fd, body_start ? body_start + 4 : "");
            } else if (strncmp(req, "GET", 3) == 0 && strstr(req, "wsdl")) {
                handle_wsdl(fd);
            } else {
                handle_not_found(fd);
            }
            free(req);
        }
        close(fd);
    }
}
