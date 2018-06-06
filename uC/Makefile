# tools
TOOL	= /opt/yagarto/bin/arm-none-eabi
#TOOL	= arm-linux
BT2	= ../bootloader2103/bt2.exe -c
#BT2	= ../bootloader2103/bt2

CC      = $(TOOL)-gcc
LD      = $(TOOL)-ld -v
AR      = $(TOOL)-ar
AS      = $(TOOL)-as
CP      = $(TOOL)-objcopy
OD	= $(TOOL)-objdump

# select the proper linker script
#LNKSC	= linker_flash.ld
LNKSC	= linker_ram.ld
#LNKSC	= linker_dram.ld

CFLAGS  = -I./ -fno-common -O2 -g -mtune=arm7tdmi -nostartfiles -static -DVERSION=$(VCODE)
AFLAGS  = -ahls -mapcs-32
CPFLAGS = -O ihex
ODFLAGS	= -x --syms

all:	code.bin code.hex
#	echo $(OSTYPE)
	$(BT2) -l code.bin

clean:
	-rm -f crt.lst main.lst crt.o main.o code.elf code.bin code.hex

code.bin:	code.elf
	$(CP) -O binary $< $@

code.hex:	code.elf
	$(CP) -O ihex $< $@

#code.elf: crt.S main.S linker_ram.ld Makefile
code.elf: crt.S main.c linker_ram.ld Makefile
	$(CC) $(CFLAGS) -Wl,-T$(LNKSC) -o $@ crt.S main.c

burn: crt.S main.c linker_flash.ld Makefile
	$(CC) $(CFLAGS) -DREAL -Wl,-Tlinker_flash.ld -o code.elf crt.S main.c 
	$(CP) -O ihex code.elf code.hex	
	../lpc21isp/lpc21isp -control -term code.hex /dev/ttyUSB0 115200 14746
	
