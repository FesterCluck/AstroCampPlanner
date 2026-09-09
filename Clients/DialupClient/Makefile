CC ?= cc
CFLAGS ?= -std=c11 -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE -Wall -Wextra -O2
SHARED_DIR = ../../TestServers/DialupServer
CPPFLAGS = -I$(SHARED_DIR)

all: client

client: client.c client_lib.o bufio.o protocol.o cards.o
	$(CC) $(CFLAGS) -o $@ client.c client_lib.o bufio.o protocol.o cards.o

client_lib.o: client_lib.c client_lib.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ client_lib.c

bufio.o: $(SHARED_DIR)/bufio.c $(SHARED_DIR)/bufio.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $(SHARED_DIR)/bufio.c

protocol.o: $(SHARED_DIR)/protocol.c $(SHARED_DIR)/protocol.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $(SHARED_DIR)/protocol.c

cards.o: $(SHARED_DIR)/cards.c $(SHARED_DIR)/cards.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $(SHARED_DIR)/cards.c

clean:
	rm -f *.o client

.PHONY: all clean
