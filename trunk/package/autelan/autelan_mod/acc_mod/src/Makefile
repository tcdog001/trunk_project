obj-m    := acc_mod.o
acc_mod-objs := check_acc.o

EXTRA_CFLAGS:=-Wall -Wno-unused -g -I.
LDFLAG:=
CC:=$(CROSS_COMPILE)gcc
RM:=rm -rf 
KERNELPATH="$(KLIB_BUILD)"

ifdef LTEFI_V2
	EXTRA_CFLAGS+=-DLTEFI_V2
endif

all:clean
	$(MAKE) -C $(KERNELPATH) SUBDIRS=$(shell pwd) modules
clean:
	$(RM) $(TARGET) *.o *.ko *.mod.c .*.cmd *.order *.symvers .tmp_versions
