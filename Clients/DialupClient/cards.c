#include "cards.h"

#include <string.h>

void normalize_card(const char *in, char *out) {
    size_t j = 0;
    for (size_t i = 0; in[i]; i++) {
        if (in[i] == ' ' || in[i] == '-') continue;
        out[j++] = in[i];
    }
    out[j] = '\0';
}

int card_is_known(const char *card) {
    return strcmp(card, CARD_APPROVED_NUMBER) == 0 || strcmp(card, CARD_DECLINED_NUMBER) == 0;
}
