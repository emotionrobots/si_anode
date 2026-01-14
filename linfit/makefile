CC      := gcc
CFLAGS  := -std=c11 -O2 -Wall -Wextra -Wpedantic -Werror
LDFLAGS := -lm

.PHONY: all clean test

all: test_linfit

linfit.o: linfit.c linfit.h
	$(CC) $(CFLAGS) -c linfit.c -o linfit.o

test_linfit.o: test_linfit.c linfit.h
	$(CC) $(CFLAGS) -c test_linfit.c -o test_linfit.o

test_linfit: linfit.o test_linfit.o
	$(CC) $(CFLAGS) linfit.o test_linfit.o -o test_linfit $(LDFLAGS)

test: test_linfit
	./test_linfit

clean:
	rm -f *.o test_linfit

