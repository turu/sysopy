CC=gcc
CFLAGS=-Wall -lrt

all: client server

client: client.o
	$(CC) $(CFLAGS) -o $@.out $^

server: server.o
	$(CC) $(CFLAGS) -o $@.out $^

clean:
	rm -rvf *.out
	rm -rvf *.o
