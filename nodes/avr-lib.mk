
CXXFLAGS = -g -mmcu=$(MCU) -Os -fdata-sections -ffunction-sections -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums -fno-inline-small-functions -DF_CPU=$(F_CPU)
INCLUDES = 
LFLAGS   = -g -mmcu=$(MCU) -Wl,--relax
LIBS =

# SRC = $(wildcard *.cc) $(EXTSRC)
OBJ = $(SRC:.cc=.o) $(SRC:.cpp=.o)

.PHONY: all clean fuse program

all: $(TARGET).a

clean:
	rm -f $(TARGET).a $(OBJ)

$(TARGET).a: $(OBJ)
	$(AR) rcs $@ $^
#	$(LD) -o $@ $(OBJ) $(LFLAGS) $(LIBS)
	$(SIZE) -t $@

.cc.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

