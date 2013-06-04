CC = gcc
CFLAGS = -Wall -lpthread

all: zad3a

zad3a: zad3a.o
	$(CC) -o $@.out $^ $(CFLAGS)

clean :
	rm -rvf *.out
	rm -rvf *.o
