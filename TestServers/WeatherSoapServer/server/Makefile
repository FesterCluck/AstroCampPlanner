CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -O2

all: weather_server

weather_server: weather_server.c
	$(CC) $(CFLAGS) -o $@ weather_server.c

clean:
	rm -f weather_server server.log

.PHONY: all clean
