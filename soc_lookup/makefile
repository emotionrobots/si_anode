CC := gcc
CFLAGS := -std=c11 -O2 -Wall -Wextra -Werror -pedantic
LDFLAGS :=

TARGET := test_soc_ocv_lookup
OBJS := soc_ocv_lookup.o test_soc_ocv_lookup.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

soc_ocv_lookup.o: soc_ocv_lookup.c soc_ocv_lookup.h
	$(CC) $(CFLAGS) -c soc_ocv_lookup.c

test_soc_ocv_lookup.o: test_soc_ocv_lookup.c soc_ocv_lookup.h
	$(CC) $(CFLAGS) -c test_soc_ocv_lookup.c

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: all test clean

