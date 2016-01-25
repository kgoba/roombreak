PROGRAMMER = usbasp
F_CPU 	= 16000000
MCU 	= atmega328p

CXX = avr-g++
LD  = avr-g++
OBJDUMP = avr-objdump
OBJCOPY = avr-objcopy
SIZE	= avr-size

TARGET 	 = pbx
SRC 	 = pbx.cc user.cc line.cc ../Common/util.cc

CXXFLAGS = -g -mmcu=$(MCU) -Os -DF_CPU=$(F_CPU)
INCLUDES = -I../
LFLAGS   = -g -mmcu=$(MCU)
LIBS =

OBJ = $(SRC:.cc=.o)

all: $(TARGET).hex $(TARGET).lst

clean:
	rm -f $(TARGET).hex $(TARGET).lst $(TARGET).elf $(OBJ)

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -j .text -j .data -O ihex $(TARGET).elf $(TARGET).hex

$(TARGET).lst: $(TARGET).elf
	$(OBJDUMP) -h -S $(TARGET).elf >$(TARGET).lst

$(TARGET).elf: $(OBJ)
	$(LD) -o $@ $(OBJ) $(LFLAGS) $(LIBS)
	$(SIZE) $@

.cc.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

fuse:
	avrdude -p $(MCU) -c $(PROGRAMMER) -U lfuse:w:$(LFUSE):m -U hfuse:w:$(HFUSE):m -U efuse:w:$(XFUSE):m

program: $(TARGET).hex
	avrdude -p $(MCU) -c $(PROGRAMMER) -U flash:w:$(TARGET).hex

