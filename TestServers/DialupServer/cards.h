#ifndef CARDS_H
#define CARDS_H

#define CARD_APPROVED_NUMBER "4242424242424242"
#define CARD_APPROVED_EXPIRY "01/99"
#define CARD_APPROVED_CVV    "123"

#define CARD_DECLINED_NUMBER "4000000000000002"
#define CARD_DECLINED_EXPIRY "01/99"
#define CARD_DECLINED_CVV    "123"

void normalize_card(const char *in, char *out);

int card_is_known(const char *card);

#endif
