CC = gcc
CFLAGS = -std=c11 -g -O0 -Wall -Wextra -Werror

SRCS = task21_test.c task21_crc.c task21_frame.c task21_parser.c task21_protocol.c task21_ring.c

TARGET = task21_test

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

run: $(TARGET)
	.\$(TARGET).exe

clean:
	rm -f $(TARGET)

.PHONY: all clean