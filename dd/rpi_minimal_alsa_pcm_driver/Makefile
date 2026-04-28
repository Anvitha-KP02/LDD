# Out-of-tree build for alsa_pcm_driver.ko
#
# Usage (on target or cross-build):
#   make -C /lib/modules/$(uname -r)/build M=$(PWD) modules
#
# Cross (example):
#   make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- \
#        -C ~/rpi-linux M=$(PWD) modules

ifneq ($(KERNELRELEASE),)
obj-m += alsa_pcm_driver.o

else
KDIR ?= /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

endif
