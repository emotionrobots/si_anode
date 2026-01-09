CC      := gcc
CFLAGS  := -std=c11 -Wall -Wextra -Wpedantic -O2
LDFLAGS :=

TARGET  := fifo_test
OBJS    := fifo.o fifo_test.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

fifo.o: fifo.c fifo.h
	$(CC) $(CFLAGS) -c fifo.c

fifo_test.o: fifo_test.c fifo.h
	$(CC) $(CFLAGS) -c fifo_test.c

clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: all clean

