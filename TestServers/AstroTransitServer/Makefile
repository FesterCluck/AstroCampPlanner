CC ?= cc
CFLAGS ?= -std=gnu11 -D_DEFAULT_SOURCE -Wall -Wextra -O2
LDLIBS = -lm

all: astro_server

astro_server: astro_server.c transit.c constellations.c
	$(CC) $(CFLAGS) -o $@ astro_server.c transit.c constellations.c $(LDLIBS)

clean:
	rm -f astro_server server.log

.PHONY: all clean
