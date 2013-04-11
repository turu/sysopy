CC = gcc
SHELL = /bin/bash
CFLAGS = -Wall
SOURCES = zad1.c
OBJ =  $(SOURCES:.c=.o)
.PHONY = clean all zad1

ifndef MODE
	MODE = FORK
endif
ifeq ($(MODE), FORK)
	CFLAGS += -D FORK
endif
ifeq ($(MODE), VFORK)
	CFLAGS += -D VFORK
endif
ifeq ($(MODE), CLONE)
	CFLAGS += -D CLONE
endif
ifeq ($(MODE), VCLONE)
	CFLAGS += -D VCLONE
endif

all: zad1

zad1: $(OBJ)
	$(CC) $(CFLAGS)  $^ -o $@.out	

clean:
	rm -f *.out
	rm -f *.o
