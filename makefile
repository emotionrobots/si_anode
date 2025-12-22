CC      := gcc
CFLAGS  := -DREALTIME -Wall -Wextra -std=c11 -O2 -pthread
#CFLAGS  := -Wall -Wextra -std=c11 -O2 -pthread
LDFLAGS := -pthread -lm -lc

TARGET  := app
OBJS    := system.o fgic.o batt.o ecm.o itimer.o app.o flash_params.o sim.o util.o

.PHONY: all clean run

all: $(TARGET)

util.o: util.c util.h
	$(CC) $(CFLAGS) -c $< -o $@

batt.o: batt.c batt.h
	$(CC) $(CFLAGS) -c $< -o $@

fgic.o: fgic.c fgic.h
	$(CC) $(CFLAGS) -c $< -o $@

ecm.o: ecm.c ecm.h
	$(CC) $(CFLAGS) -c $< -o $@

itimer.o: itimer.c itimer.h
	$(CC) $(CFLAGS) -c $< -o $@

flash_params.o: flash_params.c flash_params.h
	$(CC) $(CFLAGS) -c $< -o $@

system.o: system.c system.h
	$(CC) $(CFLAGS) -c $< -o $@

sim.o: sim.c sim.h
	$(CC) $(CFLAGS) -c $< -o $@

app.o: app.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(OBJS)

