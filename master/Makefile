TARGET 	= master
SRC 	= gpio.cc master.cc
CXXFLAGS = -g -O3
INCLUDES = 
LFLAGS = -g -L/usr/local/lib
LIBS = 

OBJ = $(SRC:.cc=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) -o $(TARGET) $(OBJ) $(LIBS) $(LFLAGS)

.cc.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

