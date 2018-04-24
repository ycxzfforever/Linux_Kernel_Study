# The path of kernel source code
INCLUDEDIR = /usr/src/linux-2.4/include/

# Compiler
CC = gcc

# Options
CFLAGS = -D__KERNEL__ -DMODULE -O -Wall -I$(INCLUDEDIR)

# Target
OBJS = timer.o

all: $(OBJS)

$(OBJS): timer.c
	$(CC) $(CFLAGS) -c $<

install:
	insmod $(OBJS)

uninstall:
	rmmod timer

.PHONY: clean
clean:
	rm -f *.o
