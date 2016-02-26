PROG=usbasp
#PROG=avrisp
#PROGOPT=-P /dev/tty.usbmodem26241 -b 19200
DEV=m328p
LFUSE=0xFF
HFUSE=0xDA
XFUSE=0x05

all: tsb_mega.hex

tsb_mega.hex: tsb_mega.asm
	gavrasm tsb_mega.asm

program: tsb_mega.hex
	avrdude -p ${DEV} -c ${PROG} $(PROGOPT) -u -U flash:w:tsb_mega.hex
	#avrdude -p ${DEV} -c ${PROG} $(PROGOPT) -u -U lock:w:0x2F:m

fuse:
	avrdude -p $(DEV) -c $(PROG) $(PROGOPT) -U lfuse:w:$(LFUSE):m -U hfuse:w:$(HFUSE):m -U efuse:w:$(XFUSE):m 

check:
	avrdude -p m328p -c $(PROG) $(PROGOPT) -u -U lock:r:lockbits.hex:h	
