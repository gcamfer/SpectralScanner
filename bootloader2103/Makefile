TOOL	= arm-linux
CC      = $(TOOL)-gcc
LD      = $(TOOL)-ld -v
AR      = $(TOOL)-ar
AS      = $(TOOL)-as
CP      = $(TOOL)-objcopy
OD	= $(TOOL)-objdump

CFLAGS  = -I./ -fno-common -O2 -g -mtune=arm7tdmi -nostartfiles -static
AFLAGS  = -ahls -mapcs-32
CPFLAGS = -O ihex
ODFLAGS	= -x --syms

all:	bt2

clean:
	-rm -f bt2 boot2.elf boot2.bin boot2.h 

bt2:	bt2.c boot2.h
	gcc -w -O2 -o $@ $<

bt2_cygwin.exe: bt2_cygwin.c boot2.h
	gcc -w -O2 -o $@ $<
    
boot2.h:	boot2.bin hexador
	./hexador boot2.bin >$@

hexador:	hexador.c
	gcc -O2 -o $@ $<

boot2.bin:	boot2.elf
	$(CP) -O binary $< $@

boot2.elf:	boot2.s
	$(CC) $(CFLAGS) -o $@ $<

