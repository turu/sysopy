CC=gcc
CFLAGS = -Wall

SRC1 = pisarz.c
SRC2 = czytelnik.c
HEAD = types.h

.PHONY : all
all : pisarz czytelnik

pisarz : pisarz.o
	$(CC) $(CFLAGS) -o $@ $^ -lrt -pthread

pisarz.o : $(SRC1) $(HEAD)
	$(CC) $(CFLAGS) -c -o $@ $(SRC1)
	
czytelnik : czytelnik.o
	$(CC) $(CFLAGS) -o $@ $^ -lrt -pthread

czytelnik.o : $(SRC2) $(HEAD)
	$(CC) $(CFLAGS) -c -o $@ $(SRC2)

.PHONY : clean
clean :
	rm -f *.o
	
.PHONY : uninstall
uninstall :
	rm -f pisarz czytelnik


