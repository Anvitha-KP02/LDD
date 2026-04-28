CC ?= gcc
CFLAGS += -Wall -Wextra -O2
LDFLAGS += -lasound -lpthread

TARGET := alsa_display_sync
SRC := alsa_display_sync.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET)
