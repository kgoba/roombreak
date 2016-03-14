TARGET 	= master
SRC 	= master.cc
CXXFLAGS = -g -O3
INCLUDES = 
LFLAGS = -g -L/usr/local/lib
LIBS = -lwiringPi

OBJ = $(SRC:.cc=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) -o $(TARGET) $(OBJ) $(LFLAGS) $(LIBS)

.cc.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

