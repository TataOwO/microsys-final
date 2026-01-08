obj-m := demo.o

kernel_dir := /usr/src/linux-headers-4.9.201-tegra-ubuntu18.04_aarch64/kernel-4.9

PWD := $(shell pwd)

all:
	make -C $(kernel_dir) SUBDIRS=$(PWD)
clean:
	rm -f *.o *.ko *.mod.c
.PHONY:
	clean
