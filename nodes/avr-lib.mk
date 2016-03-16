
CXXFLAGS = -g -mmcu=$(MCU) -Os -fdata-sections -ffunction-sections -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums -fno-inline-small-functions -DF_CPU=$(F_CPU)
INCLUDES = 
LFLAGS   = -g -mmcu=$(MCU) -Wl,--relax
LIBS =

# SRC = $(wildcard *.cc) $(EXTSRC)
OBJ = $(SRC:.cc=.o)

.PHONY: all clean fuse program

all: $(TARGET).a
	@echo "  Library built successfully"

clean:
	@echo "  Cleaning all..."
	@rm -f $(TARGET).a $(OBJ)

$(TARGET).a: $(OBJ)
	@echo "  Linking..."
	@$(AR) rcs $@ $^
#	$(LD) -o $@ $(OBJ) $(LFLAGS) $(LIBS)
	@$(SIZE) -t $@

.cc.o:
	@echo "  Compiling $<..."
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

