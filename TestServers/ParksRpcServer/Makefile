CC ?= cc
CFLAGS ?= -std=gnu11 -D_DEFAULT_SOURCE -Wall -Wextra -O2 $(shell pkg-config --cflags libtirpc)
GENFLAGS = $(CFLAGS) -Wno-unused-variable -Wno-cast-function-type
LDLIBS = $(shell pkg-config --libs libtirpc) -lm

COMMON_OBJS = park.o parks_xdr.o
SERVER_OBJS = parks_svc.o parks_proc.o

all: rpc_server

rpc_server: $(SERVER_OBJS) $(COMMON_OBJS)
	$(CC) $(CFLAGS) -o $@ $(SERVER_OBJS) $(COMMON_OBJS) $(LDLIBS)

parks_xdr.o: parks_xdr.c parks.h
	$(CC) $(GENFLAGS) -c -o $@ parks_xdr.c

parks_svc.o: parks_svc.c parks.h
	$(CC) $(GENFLAGS) -c -o $@ parks_svc.c

%.o: %.c park.h parks.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o rpc_server server.log

.PHONY: all clean
