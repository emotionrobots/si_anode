CC      := gcc
#CFLAGS  := -DREALTIME -Wall -Wextra -std=c11 -O2 -pthread
CFLAGS  += $(shell sdl2-config --cflags) -Wall -Wextra -std=c11 -O2 -pthread
LDFLAGS := $(shell sdl2-config --libs) -lSDL2_ttf -pthread -lm -lc

TARGET  := app
OBJS    := system.o fgic.o batt.o ecm.o itimer.o app.o flash_params.o sim.o util.o \
	   menu.o app_menu.o scope_plot.o ukf.o soc_ocv_lookup.o
INCS 	:= *.h 


.PHONY: all clean run

all: $(TARGET)

fifo.o: fifo.c $(INCS)
	$(CC) $(CFLAGS) -c $< -o $@

util.o: util.c $(INCS)
	$(CC) $(CFLAGS) -c $< -o $@

soc_ocv_lookup: soc_ocv_lookup.c $(INC)
	$(CC) $(CFLAGS) -c $< -o $@

ukf.o: ukf.c $(INCS)
	$(CC) $(CFLAGS) -c $< -o $@

batt.o: batt.c $(INCS)
	$(CC) $(CFLAGS) -c $< -o $@

fgic.o: fgic.c $(INCS)
	$(CC) $(CFLAGS) -c $< -o $@

ecm.o: ecm.c $(INCS)
	$(CC) $(CFLAGS) -c $< -o $@

itimer.o: itimer.c $(INCS)
	$(CC) $(CFLAGS) -c $< -o $@

flash_params.o: flash_params.c $(INCS)
	$(CC) $(CFLAGS) -c $< -o $@

system.o: system.c $(INCS)
	$(CC) $(CFLAGS) -c $< -o $@

sim.o: sim.c $(INCS)
	$(CC) $(CFLAGS) -c $< -o $@

menu.o: menu.c $(INCS)
	$(CC) $(CFLAGS) -c $< -o $@

app_menu.o: app_menu.c $(INCS)
	$(CC) $(CFLAGS) -c $< -o $@

scope_plot.o: scope_plot.c $(INCS)
	$(CC) $(CFLAGS) -c $< -o $@

app.o: app.c menu.h $(INCS)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS) $(INCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(OBJS) *.csv

