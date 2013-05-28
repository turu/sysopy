CC=gcc
CFLAGS = -Wall

.PHONY : all
all : pisarz czytelnik

pisarz : pisarz.o
	$(CC) $(CFLAGS) -o $@.out $^ -lrt -pthread

czytelnik : czytelnik.o
	$(CC) $(CFLAGS) -o $@.out $^ -lrt -pthread

clean :
	rm -rvf *.o
	rm -rvf *.out

