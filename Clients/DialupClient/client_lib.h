#ifndef CLIENT_LIB_H
#define CLIENT_LIB_H

#include <stddef.h>

typedef enum {
    AUTH_APPROVED,
    AUTH_DECLINED,
    AUTH_REJECTED,
    AUTH_NO_CARRIER,
    AUTH_ERROR
} auth_status_t;

typedef struct {
    auth_status_t status;
    char code[64];
    char reason[128];
    char auth_code[64];
} auth_result_t;

auth_result_t authorize(const char *port_path, const char *card, const char *expiry,
                         const char *cvv, const char *amount,
                         char *transcript, size_t transcript_len);

#endif
