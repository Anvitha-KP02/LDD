# Out-of-tree module for Raspberry Pi kernel 6.12.x (e.g. 6.12.62+rpt-rpi-v8)
# make -C /lib/modules/$(uname -r)/build M=$(PWD) modules
# sudo make -C ... modules_install  (optional)

obj-m += ili9225_v4l2.o

KDIR ?= /lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
