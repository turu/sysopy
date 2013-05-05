CC=gcc
CFLAGS=-Wall

all: policz dodaj_plik

policz: policz.c
	$(CC) $(CFLAGS) policz.c -o $@.out

dodaj_plik: dodaj_plik.c
	$(CC) $(CFLAGS) -O1 $@.c -o $@.out

clean:
	rm -rvf *.out
	rm -rvf *.o
	rm -rvf wyniki.log
