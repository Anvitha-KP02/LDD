# Out-of-tree build for audio_driver.ko
# On Raspberry Pi: use kernel source tree or RPi kernel headers.

obj-m += audio_driver.o

KDIR ?= /lib/modules/$(shell uname -r)/build

PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

.PHONY: all clean
