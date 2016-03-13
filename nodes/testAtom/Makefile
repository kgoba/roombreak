PROGRAMMER = usbasp
F_CPU 	= 16000000
MCU 	= atmega328p
LFUSE 	= 0xE2
HFUSE 	= 0xDD
XFUSE 	= 0xFF

CC = avr-gcc
CXX = avr-g++
LD = avr-gcc
OBJDUMP = avr-objdump
OBJCOPY = avr-objcopy

TARGET 	= testAtom
SRC 	= main.cc
CXXFLAGS = -g -mmcu=$(MCU) -Os -DF_CPU=$(F_CPU) -fdata-sections -ffunction-sections -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums -fno-inline-small-functions
INCLUDES = -I../../atomthreads/kernel -I../../atomthreads/ports/avr
LFLAGS = -g -mmcu=$(MCU) -L. -Wl,--relax,--gc-sections
LIBS = -latomthreads-avr 

OBJ = $(SRC:.cc=.o)

.PHONY: all clean fuse program

all: $(TARGET).hex $(TARGET).lst
	@avr-size $(TARGET).elf

$(TARGET).hex: $(TARGET).elf
	@$(OBJCOPY) -j .text -j .data -O ihex $(TARGET).elf $(TARGET).hex

$(TARGET).lst: $(TARGET).elf
	@$(OBJDUMP) -h -S $(TARGET).elf >$(TARGET).lst

$(TARGET).elf: $(OBJ)
	@$(LD) -o $(TARGET).elf $(OBJ) $(LFLAGS) $(LIBS)

.c.o:
	@echo "Compiling $<"
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

.cc.o:
	@echo "Compiling $<"
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@


clean:
	@rm -f $(TARGET).hex $(TARGET).elf $(TARGET).lst $(OBJ)

fuse:
	@avrdude -p $(MCU) -c $(PROGRAMMER) -U lfuse:w:$(LFUSE):m -U hfuse:w:$(HFUSE):m -U efuse:w:$(XFUSE):m

program: $(TARGET).hex
	@avrdude -p $(MCU) -c $(PROGRAMMER) -U flash:w:$(TARGET).hex

