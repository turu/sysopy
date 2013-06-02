CC=gcc
CFLAGS = -Wall -lpthread

SRC1 = serwer.c
SRC2 = klient.c
HEAD = structures.h

.PHONY : all
all : serwer klient

serwer : serwer.o
	$(CC) $(CFLAGS) -o $@ $^

serwer.o : $(SRC1) $(HEAD)
	$(CC) $(CFLAGS) -c -o $@ $(SRC1)
	
klient : klient.o
	$(CC) $(CFLAGS) -o $@ $^

klient.o : $(SRC2) $(HEAD)
	$(CC) $(CFLAGS) -c -g -o $@ $(SRC2)

.PHONY : clean
clean :
	rm -f *.o
	
.PHONY : uninstall
uninstall :
	rm -f serwer klient



