ifneq ($(KERNELRELEASE),)

# kbuild part of makefile
obj-m += d_usbhost.o

else

# normal makefile

TARGET = d_usbhost.ko

CONF = Linuxmod
ARCH = AM335x

include ../../open_first/rules.mk

endif
