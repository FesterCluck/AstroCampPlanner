#ifndef BUFIO_H
#define BUFIO_H

#include <stddef.h>

typedef struct {
    int fd;
    double default_timeout;
    unsigned char buf[4096];
    size_t start, len;
} bufio_t;

void bufio_init(bufio_t *b, int fd, double default_timeout);

size_t bufio_read(bufio_t *b, void *out, size_t n);

unsigned char *bufio_read_until(bufio_t *b, const unsigned char *term, size_t term_len,
                                 double timeout, size_t *out_len);

int bufio_write(bufio_t *b, const void *data, size_t n);

#endif
