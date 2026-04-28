# SPI framebuffer video player — FFmpeg userspace decode only.
#
# Raspberry Pi OS:
#   sudo apt install libavformat-dev libavcodec-dev libswscale-dev libavutil-dev

APP        := video_player
SRC        := video_player.c
CC         ?= gcc
CFLAGS     += -std=c11 -Wall -Wextra -O2 -D_GNU_SOURCE
PKG_LIBS   := libavformat libavcodec libswscale libavutil
PKG_CFLAGS := $(shell pkg-config --cflags $(PKG_LIBS) 2>/dev/null)
PKG_LIBS_OUT := $(shell pkg-config --libs $(PKG_LIBS) 2>/dev/null)
ifeq ($(strip $(PKG_LIBS_OUT)),)
$(error Run: sudo apt install pkg-config libavformat-dev libavcodec-dev libswscale-dev libavutil-dev)
endif
CFLAGS     += $(PKG_CFLAGS)
LDFLAGS    += $(PKG_LIBS_OUT)
# libm sometimes required for pkg-config order on minimal setups
LDFLAGS    += -lm

.PHONY: all clean

all: $(APP)

$(APP): $(SRC)
	$(CC) $(CFLAGS) -o $@ $(SRC) $(LDFLAGS)

clean:
	rm -f $(APP)
