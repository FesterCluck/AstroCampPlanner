#include "bufio.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <time.h>
#include <unistd.h>

static double now_monotonic(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

void bufio_init(bufio_t *b, int fd, double default_timeout) {
    b->fd = fd;
    b->default_timeout = default_timeout;
    b->start = 0;
    b->len = 0;
}

static int bufio_fill(bufio_t *b, double deadline, int has_deadline) {
    if (b->start > 0) {
        memmove(b->buf, b->buf + b->start, b->len);
        b->start = 0;
    }
    if (b->len == sizeof(b->buf)) {
        return 0;
    }

    for (;;) {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(b->fd, &rfds);
        struct timeval tv, *tvp = NULL;
        if (has_deadline) {
            double remaining = deadline - now_monotonic();
            if (remaining <= 0) return 0;
            tv.tv_sec = (time_t)remaining;
            tv.tv_usec = (suseconds_t)((remaining - (double)tv.tv_sec) * 1e6);
            tvp = &tv;
        }
        int rc = select(b->fd + 1, &rfds, NULL, NULL, tvp);
        if (rc < 0) {
            if (errno == EINTR) continue;
            return 0;
        }
        if (rc == 0) return 0;

        ssize_t n = read(b->fd, b->buf + b->len, sizeof(b->buf) - b->len);
        if (n < 0) {
            if (errno == EINTR) continue;
            return 0;
        }
        if (n == 0) return 0;
        b->len += (size_t)n;
        return 1;
    }
}

size_t bufio_read(bufio_t *b, void *out, size_t n) {
    int has_deadline = (b->default_timeout >= 0);
    double deadline = has_deadline ? now_monotonic() + b->default_timeout : 0;

    while (b->len < n) {
        if (!bufio_fill(b, deadline, has_deadline)) break;
    }
    size_t take = b->len < n ? b->len : n;
    memcpy(out, b->buf + b->start, take);
    b->start += take;
    b->len -= take;
    return take;
}

unsigned char *bufio_read_until(bufio_t *b, const unsigned char *term, size_t term_len,
                                 double timeout, size_t *out_len) {
    int has_deadline = (timeout >= 0);
    double deadline = has_deadline ? now_monotonic() + timeout : 0;

    for (;;) {
        if (b->len >= term_len) {
            for (size_t i = 0; i + term_len <= b->len; i++) {
                if (memcmp(b->buf + b->start + i, term, term_len) == 0) {
                    size_t total = i + term_len;
                    unsigned char *result = malloc(total);
                    memcpy(result, b->buf + b->start, total);
                    b->start += total;
                    b->len -= total;
                    *out_len = total;
                    return result;
                }
            }
        }
        if (!bufio_fill(b, deadline, has_deadline)) return NULL;
    }
}

int bufio_write(bufio_t *b, const void *data, size_t n) {
    const unsigned char *p = data;
    size_t written = 0;
    while (written < n) {
        ssize_t w = write(b->fd, p + written, n - written);
        if (w < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        written += (size_t)w;
    }
    return 0;
}
