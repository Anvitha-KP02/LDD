CC ?= gcc
CFLAGS ?= -O2 -Wall -Wextra -Wpedantic

TARGET := audio_format_player

all: $(TARGET)

$(TARGET): main.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGET)

.PHONY: all clean
