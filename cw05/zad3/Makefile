CC = gcc
SHELL = /bin/bash
CFLAGS = -Wall
.PHONY = clean all

all: zad3aSend zad3aRec zad3bSend zad3bRec zad3cSend zad3cRec

zad3aSend: zad3aSend.c
	$(CC) $(CFLAGS)  $^ -o $@.out
zad3bSend: zad3bSend.c
	$(CC) $(CFLAGS)  $^ -o $@.out
zad3cSend: zad3cSend.c
	$(CC) $(CFLAGS)  $^ -o $@.out
zad3aRec: zad3aRec.c
	$(CC) $(CFLAGS)  $^ -o $@.out
zad3bRec: zad3bRec.c
	$(CC) $(CFLAGS)  $^ -o $@.out
zad3cRec: zad3cRec.c
	$(CC) $(CFLAGS)  $^ -o $@.out

clean:
	rm -rfv *.out
	rm -rfv *.o
