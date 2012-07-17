#
# Makefile for AVR ATMega16
# Good example of makefiles for AVR
# can be taken from
# http://www.obdev.at/products/vusb/index.html
# e.g. this: examples/hid-mouse/firmware/Makefile
#
CC=avr-gcc
CXX=avr-g++
MMCU=atmega16

F_CLK=F_CLK=8000000UL
F_CPU=F_CPU=8000000UL
FLAGS=-c -O3 -Wall -gdwarf-2 -mmcu=$(MMCU) -D$(F_CLK) -D$(F_CPU)

CFLAGS=-std=gnu99 $(FLAGS)
CXXFLAGS=$(FLAGS)
LFLAGS=-mmcu=$(MMCU)

all: fedorball.elf

fedorball.elf: main.o Tlc5940.o
	$(CC) $(LFLAGS) main.o Tlc5940.o -o fedorball.elf

Tlc5940.o:
	$(CXX) $(CXXFLAGS) tlc5940/Tlc5940.cpp

# Flash burning
# to avoid sudo place udev rule for USBASP as /etc/udev/rules.d/usbasp.rules:
# SUBSYSTEMS=="usb", ATTRS{idVendor}=="16c0", ATTRS{idProduct}=="05dc", GROUP="users", MODE="0666"
flash: all
	avr-objcopy -j .text -j .data -O ihex fedorball.elf fedorball.hex
	avrdude -c usbasp -p m16 -u -U flash:w:fedorball.hex

# Read fuse.
# To write fuse back use this cmd:
#   HIGH FUSE
# 0b10011001 (0x99) - shipped
# 0b11011001 (0xD9) - disabled JTAG
#
#   LOW FUSE
# 0b11100001 (0xE1) - shipped
# 0b11100100 (0xE4) - 8MHz internal oscillator (CKSEL3..0 = 0100)
#
# avrdude -c usbasp -p m16 -U hfuse:w:0xD9:m
# avrdude -c usbasp -p m16 -U lfuse:w:0xE4:m

fuse:
	avrdude -c usbasp -p m16 -U lfuse:r:lfuse.txt:r
	avrdude -c usbasp -p m16 -U hfuse:r:hfuse.txt:r
	od -d lfuse.txt  | head -1 | sed -e 's/0000000 *//' | xargs -i perl -e '$$str=unpack("B32", pack("N",{})); $$str =~ s/.*([01]{4})([01]{4})$$/$$1 $$2/; print "FUSE Low:  $$str\n";'
	od -d hfuse.txt  | head -1 | sed -e 's/0000000 *//' | xargs -i perl -e '$$str=unpack("B32", pack("N",{})); $$str =~ s/.*([01]{4})([01]{4})$$/$$1 $$2/; print "FUSE High: $$str\n";'

clean:
	rm -rf *o *~ fedorball.elf fedorball.hex
