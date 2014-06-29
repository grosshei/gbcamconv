CFLAGS+=$(shell pkg-config --cflags libpng) -g -std=c99
LDFLAGS+=$(shell pkg-config --libs libpng)

all:	gbcamconv

gbcamconv:	gbcamconv.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
