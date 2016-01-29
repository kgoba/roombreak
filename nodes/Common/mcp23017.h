#pragma once

#include "types.h"

class MCP23017 {
public:
  MCP23017(byte address = 0);
  
  void setup(word mode = 0xFFFF);
  
  byte readA();
  byte readB();
  word read();

  void writeA(byte value);
  void writeB(byte value);
  void write(word value);

  void setMode(word mode);
  
private:
  byte    _address;
  byte    _status;
  
  byte readRegister(byte addr);
  void writeRegister(byte addr, byte value);
};
