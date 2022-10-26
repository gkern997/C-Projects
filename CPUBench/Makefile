CC=gcc
CFLAGS=-Wall
pthread=-lpthread -Ofast

build: cpubench

test-cpubench: cpubench
	./runbench.sh

cpubench: cpubench.c
	$(CC) $(CFLAGS) -o cpubench $< $(pthread)

clean:
	rm -rf cpubench

