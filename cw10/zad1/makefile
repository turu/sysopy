CC=gcc
CFLAGS = -Wall -lpthread

all : server client

server : server.o
	$(CC) $^ $(CFLAGS) -o $@.out
	
client : client.o
	$(CC) $^ $(CFLAGS) -o $@.out

clean :
	rm -rvf *.o
	rm -rvf *.out


