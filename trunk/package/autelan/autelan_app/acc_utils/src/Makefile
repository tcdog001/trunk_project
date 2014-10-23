TARGET=acc_setget acc_monitor 
CC:=$(CROSS_COMPILE)gcc
RM:=rm -rf 

ifdef LTEFI
	AUTE_CFLAGS+=-DLTEFI
endif
ifdef LTEFI_V2
	AUTE_CFLAGS+=-DLTEFI_V2
endif
ifdef LTEFI_V3
	AUTE_CFLAGS+=-DLTEFI_V3
endif

all:clean
	$(CC) $(AUTE_CFLAGS) -o acc_setget acc_setget.c -lubox -lubus -luci -ljson-c -lblobmsg_json -lnl-tiny
	$(CC) $(AUTE_CFLAGS) -o acc_monitor acc_monitor.c -lpthread -lubox -lubus -luci -ljson-c -lblobmsg_json -lnl-tiny
clean:
	$(RM) $(TARGET) *.o *.ko *.mod.c .*.cmd *.order *.symvers .tmp_versions $(TARGET)
