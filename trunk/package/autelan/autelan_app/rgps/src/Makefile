#
# Makefile	Main Makefile for the net-tools Package
#

CC = $(CROSS)gcc
#CFLAGS = -g

TARGET = ./rgps

%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@
#$(CC) -c $(CFLAGS) $< -o $@

SOURCES = $(wildcard *.c)
	OBJS = $(patsubst %.c,%.o,$(SOURCES))

$(TARGET) : $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) -lm -lpthread

	chmod a+x $(TARGET)

clean:
	rm -rf *.o rgps

# End of Makefile.
