#include <ctype.h>
#include <limits.h>
#include <pty.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "bufio.h"
#include "cards.h"
#include "protocol.h"

static char g_symlink[PATH_MAX];

static void on_signal(int sig) {
    (void)sig;
    unlink(g_symlink);
    _exit(0);
}

static void trim(char *s) {
    size_t len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1])) s[--len] = '\0';
    size_t start = 0;
    while (s[start] && isspace((unsigned char)s[start])) start++;
    if (start > 0) memmove(s, s + start, len - start + 1);
}

static void to_upper(char *s) {
    for (; *s; s++) *s = (char)toupper((unsigned char)*s);
}

static void authorize(const proto_frame_t *req, proto_frame_t *resp) {
    frame_clear(resp);
    if (req->count < 5 || strcmp(req->fields[0], "REQUEST") != 0 || strcmp(req->fields[1], "AUTH") != 0) {
        frame_add(resp, "RESPONSE");
        frame_add(resp, "DECLINE");
        frame_add(resp, "05");
        frame_add(resp, "INVALID REQUEST");
        return;
    }

    char card[PROTO_MAX_FIELD_LEN];
    normalize_card(req->fields[2], card);
    const char *expiry = req->fields[3];
    const char *cvv = req->fields[4];

    if (strcmp(card, CARD_APPROVED_NUMBER) == 0 && strcmp(expiry, CARD_APPROVED_EXPIRY) == 0 &&
        strcmp(cvv, CARD_APPROVED_CVV) == 0) {
        char auth_code[8];
        snprintf(auth_code, sizeof(auth_code), "%06d", rand() % 1000000);
        frame_add(resp, "RESPONSE");
        frame_add(resp, "APPROVAL");
        frame_add(resp, "00");
        frame_add(resp, auth_code);
        return;
    }
    if (strcmp(card, CARD_DECLINED_NUMBER) == 0 && strcmp(expiry, CARD_DECLINED_EXPIRY) == 0 &&
        strcmp(cvv, CARD_DECLINED_CVV) == 0) {
        frame_add(resp, "RESPONSE");
        frame_add(resp, "DECLINE");
        frame_add(resp, "05");
        frame_add(resp, "DO NOT HONOR");
        return;
    }
    if (card_is_known(card)) {
        frame_add(resp, "RESPONSE");
        frame_add(resp, "DECLINE");
        frame_add(resp, "14");
        frame_add(resp, "INVALID EXPIRY OR CVV");
        return;
    }
    frame_add(resp, "RESPONSE");
    frame_add(resp, "DECLINE");
    frame_add(resp, "14");
    frame_add(resp, "INVALID CARD NUMBER");
}

static void reply(bufio_t *port, const char *text) {
    char line[128];
    int n = snprintf(line, sizeof(line), "%s\r\n", text);
    bufio_write(port, line, (size_t)n);
    printf(">> %s\n", text);
    fflush(stdout);
}

static int read_command(bufio_t *port, const unsigned char *term, size_t term_len,
                         double timeout, char *out, size_t out_len) {
    size_t len;
    unsigned char *line = bufio_read_until(port, term, term_len, timeout, &len);
    if (!line) return 0;
    size_t clen = len >= term_len ? len - term_len : len;
    if (clen >= out_len) clen = out_len - 1;
    memcpy(out, line, clen);
    out[clen] = '\0';
    free(line);
    trim(out);
    to_upper(out);
    return 1;
}

static void run(bufio_t *port) {
    printf("ATS0=1  (modem ready)\n");
    fflush(stdout);
    int connected = 0;

    for (;;) {
        if (!connected) {
            char cmd[128];
            if (!read_command(port, (const unsigned char *)"\r", 1, -1.0, cmd, sizeof(cmd))) continue;
            if (cmd[0] == '\0') continue;
            printf("<< %s\n", cmd);
            fflush(stdout);

            if (strcmp(cmd, "AT") == 0 || strcmp(cmd, "ATZ") == 0) {
                reply(port, "OK");
            } else if (strncmp(cmd, "ATDT", 4) == 0) {
                reply(port, "CONNECT 33600");
                connected = 1;
            } else if (strncmp(cmd, "ATH", 3) == 0) {
                reply(port, "OK");
            } else {
                reply(port, "ERROR");
            }
        } else {
            proto_frame_t req, resp;
            char err[128];
            if (recv_frame(port, &req, err, sizeof(err)) != 0) {
                printf("line noise: %s\n", err);
                fflush(stdout);
                connected = 0;
                continue;
            }
            authorize(&req, &resp);
            send_frame(port, &resp);

            size_t len;
            unsigned char *esc = bufio_read_until(port, (const unsigned char *)"+++", 3, 15.0, &len);
            if (!esc) {
                printf("line dropped\n");
                fflush(stdout);
                connected = 0;
                continue;
            }
            free(esc);

            char cmd[64];
            if (read_command(port, (const unsigned char *)"\r", 1, 5.0, cmd, sizeof(cmd)) &&
                strncmp(cmd, "ATH", 3) == 0) {
                reply(port, "OK");
            }
            connected = 0;
        }
    }
}

int main(int argc, char **argv) {
    const char *symlink_arg = "modem_port";
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--symlink") == 0 && i + 1 < argc) symlink_arg = argv[++i];
    }

    if (symlink_arg[0] == '/') {
        snprintf(g_symlink, sizeof(g_symlink), "%s", symlink_arg);
    } else {
        char cwd[PATH_MAX];
        if (!getcwd(cwd, sizeof(cwd))) { perror("getcwd"); return 1; }
        snprintf(g_symlink, sizeof(g_symlink), "%s/%s", cwd, symlink_arg);
    }

    int master_fd, slave_fd;
    char slave_name[256];
    if (openpty(&master_fd, &slave_fd, slave_name, NULL, NULL) != 0) {
        perror("openpty");
        return 1;
    }

    struct termios raw;
    tcgetattr(slave_fd, &raw);
    cfmakeraw(&raw);
    tcsetattr(slave_fd, TCSANOW, &raw);

    unlink(g_symlink);
    if (symlink(slave_name, g_symlink) != 0) {
        perror("symlink");
        return 1;
    }
    printf("serial port ready: %s -> %s\n", g_symlink, slave_name);
    fflush(stdout);

    signal(SIGINT, on_signal);
    signal(SIGTERM, on_signal);

    srand((unsigned)time(NULL) ^ (unsigned)getpid());

    bufio_t port;
    bufio_init(&port, master_fd, 10.0);
    run(&port);

    return 0;
}
