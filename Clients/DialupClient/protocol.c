#include "protocol.h"

#include <stdio.h>
#include <string.h>

#define STX 0x02
#define ETX 0x03

void frame_clear(proto_frame_t *f) {
    f->count = 0;
}

void frame_add(proto_frame_t *f, const char *field) {
    if (f->count >= PROTO_MAX_FIELDS) return;
    snprintf(f->fields[f->count], PROTO_MAX_FIELD_LEN, "%s", field);
    f->count++;
}

int send_frame(bufio_t *port, const proto_frame_t *f) {
    unsigned char buf[PROTO_MAX_FIELDS * PROTO_MAX_FIELD_LEN + 3];
    size_t pos = 0;
    buf[pos++] = STX;
    for (size_t i = 0; i < f->count; i++) {
        if (i > 0) buf[pos++] = '|';
        size_t flen = strlen(f->fields[i]);
        memcpy(buf + pos, f->fields[i], flen);
        pos += flen;
    }
    buf[pos++] = ETX;

    unsigned char lrc = 0;
    for (size_t i = 1; i < pos; i++) lrc ^= buf[i];
    buf[pos++] = lrc;

    return bufio_write(port, buf, pos);
}

int recv_frame(bufio_t *port, proto_frame_t *out, char *err, size_t errlen) {
    unsigned char b;

    if (bufio_read(port, &b, 1) != 1) {
        snprintf(err, errlen, "timed out waiting for a response");
        return -1;
    }
    if (b != STX) {
        snprintf(err, errlen, "expected STX, got 0x%02x", b);
        return -1;
    }

    unsigned char data[PROTO_MAX_FIELDS * PROTO_MAX_FIELD_LEN + 2];
    size_t dlen = 0;
    for (;;) {
        if (bufio_read(port, &b, 1) != 1) {
            snprintf(err, errlen, "timed out mid-frame");
            return -1;
        }
        if (dlen >= sizeof(data)) {
            snprintf(err, errlen, "frame too long");
            return -1;
        }
        data[dlen++] = b;
        if (b == ETX) break;
    }

    unsigned char check;
    if (bufio_read(port, &check, 1) != 1) {
        snprintf(err, errlen, "timed out waiting for the LRC byte");
        return -1;
    }
    unsigned char expected = 0;
    for (size_t i = 0; i < dlen; i++) expected ^= data[i];
    if (check != expected) {
        snprintf(err, errlen, "LRC checksum mismatch (line noise)");
        return -1;
    }

    frame_clear(out);
    size_t body_len = dlen - 1;
    size_t start = 0;
    for (size_t i = 0; i <= body_len; i++) {
        if (i == body_len || data[i] == '|') {
            size_t flen = i - start;
            if (flen >= PROTO_MAX_FIELD_LEN) flen = PROTO_MAX_FIELD_LEN - 1;
            if (out->count < PROTO_MAX_FIELDS) {
                memcpy(out->fields[out->count], data + start, flen);
                out->fields[out->count][flen] = '\0';
                out->count++;
            }
            start = i + 1;
        }
    }
    return 0;
}
