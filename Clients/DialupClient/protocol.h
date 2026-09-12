#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stddef.h>

#include "bufio.h"

#define PROTO_MAX_FIELDS 8
#define PROTO_MAX_FIELD_LEN 64

typedef struct {
    char fields[PROTO_MAX_FIELDS][PROTO_MAX_FIELD_LEN];
    size_t count;
} proto_frame_t;

void frame_clear(proto_frame_t *f);
void frame_add(proto_frame_t *f, const char *field);

int send_frame(bufio_t *port, const proto_frame_t *f);

int recv_frame(bufio_t *port, proto_frame_t *out, char *err, size_t errlen);

#endif
