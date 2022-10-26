CC=gcc
CFLAGS=-Wall -O3

build: netio

test-netio: netio
	./netio ...

netio: netio.c
	$(CC) $(CFLAGS) -o netio $<

clean:
	rm -rf netio

