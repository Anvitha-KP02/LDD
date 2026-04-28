KDIR ?= /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

obj-m += rpi_alsa_raw_driver.o

USER_APP := rpi_raw_player_user
USER_SRC := rpi_raw_player_user.c

all: module user

module:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

user:
	$(CC) -Wall -Wextra -O2 -o $(USER_APP) $(USER_SRC) -lasound

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f $(USER_APP)

.PHONY: all module user clean
