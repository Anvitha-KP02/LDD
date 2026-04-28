CC ?= gcc
CFLAGS ?= -Wall -Wextra -O2 -g

all: demo_my_mmap

demo_my_mmap: demo_my_mmap.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f demo_my_mmap

.PHONY: all clean
