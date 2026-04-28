# Audio + ILI9225 LCD demo (user space)
# Requires: aplay, ffmpeg on PATH; ALSA dev + /dev/ili9225_char on target

CC ?= gcc
CFLAGS ?= -Wall -Wextra -O2 -std=c11 -D_POSIX_C_SOURCE=200809L
LDFLAGS ?= -pthread
TARGET = audio_lcd_player

.PHONY: all clean

all: $(TARGET)

$(TARGET): main.c
	$(CC) $(CFLAGS) -o $@ main.c $(LDFLAGS)

clean:
	rm -f $(TARGET)
