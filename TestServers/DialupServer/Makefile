CC ?= cc
CFLAGS ?= -std=c11 -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE -Wall -Wextra -O2
LDLIBS = -lutil

COMMON_OBJS = bufio.o protocol.o cards.o

all: server

server: server.c $(COMMON_OBJS)
	$(CC) $(CFLAGS) -o $@ server.c $(COMMON_OBJS) $(LDLIBS)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o server modem_port server.log

.PHONY: all clean
