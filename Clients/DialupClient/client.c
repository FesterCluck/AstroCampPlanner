#include <stdio.h>
#include <string.h>

#include "client_lib.h"

int main(int argc, char **argv) {
    const char *port_path = "modem_port";
    const char *amount = "0.00";
    const char *positional[3];
    int npositional = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--port") == 0 && i + 1 < argc) {
            port_path = argv[++i];
        } else if (strcmp(argv[i], "--amount") == 0 && i + 1 < argc) {
            amount = argv[++i];
        } else if (npositional < 3) {
            positional[npositional++] = argv[i];
        }
    }
    if (npositional < 3) {
        fprintf(stderr, "usage: %s [--port PATH] [--amount AMT] CARD EXPIRY CVV\n", argv[0]);
        return 4;
    }
    const char *card = positional[0], *expiry = positional[1], *cvv = positional[2];

    char transcript[1024];
    auth_result_t result = authorize(port_path, card, expiry, cvv, amount, transcript, sizeof(transcript));
    if (transcript[0]) printf("%s\n", transcript);

    switch (result.status) {
        case AUTH_APPROVED:
            printf("APPROVED  auth code %s\n", result.auth_code);
            return 0;
        case AUTH_DECLINED:
            printf("DECLINED  [%s] %s\n", result.code, result.reason);
            return 1;
        case AUTH_REJECTED:
            return 2;
        case AUTH_NO_CARRIER:
            return 3;
        default:
            return 4;
    }
}
