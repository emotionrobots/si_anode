CC ?= gcc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -pedantic
LDFLAGS ?=
SDL_CFLAGS := $(shell pkg-config --cflags sdl2 SDL2_ttf)
SDL_LIBS   := $(shell pkg-config --libs sdl2 SDL2_ttf)

OBJS_SCOPE = scope_plot.o

all: test_scope_csv test_scope_stream

scope_plot.o: scope_plot.c scope_plot.h
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -c -o $@ scope_plot.c

test_scope_csv: test_scope_csv.c $(OBJS_SCOPE)
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -o $@ test_scope_csv.c $(OBJS_SCOPE) $(SDL_LIBS) -lm

test_scope_stream: test_scope_stream.c $(OBJS_SCOPE)
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -o $@ test_scope_stream.c $(OBJS_SCOPE) $(SDL_LIBS) -lm

run_csv: test_scope_csv
	./test_scope_csv t11.csv

run_stream: test_scope_stream
	./test_scope_stream

clean:
	rm -f *.o test_scope_csv test_scope_stream

.PHONY: all clean run_csv run_stream
