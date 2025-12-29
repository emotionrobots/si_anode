CC      := gcc
CFLAGS  := -std=c11 -O2 -Wall -Wextra -Wpedantic -Werror
CFLAGS  += $(shell sdl2-config --cflags)
LDFLAGS := $(shell sdl2-config --libs) -lSDL2_ttf -lm

BIN := scope_plot_test
SRC := scope_plot.c scope_plot_test.c
OBJ := $(SRC:.c=.o)

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c scope_plot.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(BIN) $(OBJ)

run: $(BIN)
	./$(BIN)

valgrind: $(BIN)
	valgrind --leak-check=full --show-leak-kinds=all ./$(BIN)

