CFLAGS=-Wall -lpthread
CC=gcc

ifdef VER
	CFLAGS += -D $(VER)
endif

ifdef SIG
	CFLAGS += -D $(SIG)
endif

all: zad2

zad1: zad2.c
	$(CC) -o $@.out $^ $(CFLAGS)
	
clean:
	rm -rvf *.o
	rm -rvf *.out
