#Standard variables
CC=gcc $(CFLAGS)
CFLAGS=-Wall -g

#My vars
OUT=fib.out faulty.out loop.out

all: clean $(OUT)

%.out: %.c
	$(CC) $< -o $@

clean:
	rm -rfv *.out

.PHONY : all clean
