TARGET=at_ttyUSB1 at_ttyUSB3 at_ttyUSB0
CC:=$(CROSS_COMPILE)gcc
RM:=rm -rf 

ifdef LTEFI
	AUTE_CFLAGS+=-DLTEFI
endif
ifdef LTEFI_V2
	AUTE_CFLAGS+=-DLTEFI_V2
endif

all:clean
	$(CC) $(AUTE_CFLAGS) -o at_ttyUSB0 at_ttyUSB0.c -lpthread
	$(CC) $(AUTE_CFLAGS) -o at_ttyUSB1 at_ttyUSB1.c -lpthread
	$(CC) $(AUTE_CFLAGS) -o at_ttyUSB3 at_ttyUSB3.c -lpthread
clean:
	$(RM) $(TARGET) *.o *.ko *.mod.c .*.cmd *.order *.symvers .tmp_versions $(TARGET)
