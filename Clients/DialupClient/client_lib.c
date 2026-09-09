#include "client_lib.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "bufio.h"
#include "cards.h"
#include "protocol.h"

#define DIAL_NUMBER "18005551212"
#define PORT_TIMEOUT 5.0

static void append_transcript(char *transcript, size_t transcript_len, const char *line) {
    if (!transcript || transcript_len == 0) return;
    size_t used = strlen(transcript);
    if (used >= transcript_len) return;
    snprintf(transcript + used, transcript_len - used, "%s%s", used > 0 ? "\n" : "", line);
}

static int readline(bufio_t *port, char *out, size_t out_len) {
    size_t len;
    unsigned char *line = bufio_read_until(port, (const unsigned char *)"\r\n", 2, PORT_TIMEOUT, &len);
    if (!line) {
        out[0] = '\0';
        return 0;
    }
    size_t clen = len >= 2 ? len - 2 : len;
    if (clen >= out_len) clen = out_len - 1;
    memcpy(out, line, clen);
    out[clen] = '\0';
    free(line);
    return 1;
}

auth_result_t authorize(const char *port_path, const char *card, const char *expiry,
                         const char *cvv, const char *amount,
                         char *transcript, size_t transcript_len) {
    auth_result_t result;
    memset(&result, 0, sizeof(result));
    if (transcript && transcript_len > 0) transcript[0] = '\0';

    char card_norm[PROTO_MAX_FIELD_LEN];
    normalize_card(card, card_norm);
    if (!card_is_known(card_norm)) {
        char msg[192];
        snprintf(msg, sizeof(msg), "CARD REJECTED: %s is not a recognized card for this terminal", card);
        append_transcript(transcript, transcript_len, msg);
        result.status = AUTH_REJECTED;
        return result;
    }

    int fd = open(port_path, O_RDWR | O_NOCTTY);
    if (fd < 0) {
        append_transcript(transcript, transcript_len, "NO CARRIER");
        result.status = AUTH_NO_CARRIER;
        snprintf(result.reason, sizeof(result.reason), "%s", strerror(errno));
        return result;
    }

    struct termios tty;
    tcgetattr(fd, &tty);
    cfmakeraw(&tty);
    cfsetispeed(&tty, B9600);
    cfsetospeed(&tty, B9600);
    tcsetattr(fd, TCSANOW, &tty);

    bufio_t port;
    bufio_init(&port, fd, PORT_TIMEOUT);

    char line[128];

    append_transcript(transcript, transcript_len, "ATZ");
    bufio_write(&port, "ATZ\r", 4);
    readline(&port, line, sizeof(line));
    append_transcript(transcript, transcript_len, line[0] ? line : "(no response)");

    char dial_cmd[32];
    snprintf(dial_cmd, sizeof(dial_cmd), "ATDT%s", DIAL_NUMBER);
    append_transcript(transcript, transcript_len, dial_cmd);
    char dial_wire[40];
    snprintf(dial_wire, sizeof(dial_wire), "%s\r", dial_cmd);
    bufio_write(&port, dial_wire, strlen(dial_wire));
    readline(&port, line, sizeof(line));
    append_transcript(transcript, transcript_len, line[0] ? line : "(no response)");

    if (strncmp(line, "CONNECT", 7) != 0) {
        close(fd);
        result.status = AUTH_NO_CARRIER;
        return result;
    }

    proto_frame_t req, resp;
    frame_clear(&req);
    frame_add(&req, "REQUEST");
    frame_add(&req, "AUTH");
    frame_add(&req, card_norm);
    frame_add(&req, expiry);
    frame_add(&req, cvv);
    frame_add(&req, amount);

    int have_response = 0;
    char err[128];
    if (send_frame(&port, &req) != 0) {
        snprintf(err, sizeof(err), "write failed: %s", strerror(errno));
        append_transcript(transcript, transcript_len, err);
        result.status = AUTH_ERROR;
        snprintf(result.reason, sizeof(result.reason), "%s", err);
    } else if (recv_frame(&port, &resp, err, sizeof(err)) != 0) {
        char msg[192];
        snprintf(msg, sizeof(msg), "line noise: %s", err);
        append_transcript(transcript, transcript_len, msg);
        result.status = AUTH_ERROR;
        snprintf(result.reason, sizeof(result.reason), "%s", err);
    } else {
        have_response = 1;
    }

    bufio_write(&port, "+++", 3);
    struct timespec pause = {0, 200 * 1000 * 1000};
    nanosleep(&pause, NULL);
    bufio_write(&port, "ATH0\r", 5);
    readline(&port, line, sizeof(line));
    append_transcript(transcript, transcript_len, line[0] ? line : "(no response)");

    close(fd);

    if (!have_response) return result;

    if (resp.count >= 2 && strcmp(resp.fields[1], "APPROVAL") == 0) {
        result.status = AUTH_APPROVED;
        if (resp.count > 2) snprintf(result.code, sizeof(result.code), "%s", resp.fields[2]);
        if (resp.count > 3) snprintf(result.auth_code, sizeof(result.auth_code), "%s", resp.fields[3]);
    } else {
        result.status = AUTH_DECLINED;
        if (resp.count > 2) snprintf(result.code, sizeof(result.code), "%s", resp.fields[2]);
        if (resp.count > 3) snprintf(result.reason, sizeof(result.reason), "%s", resp.fields[3]);
    }
    return result;
}
