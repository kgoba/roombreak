#pragma once

#include "types.h"

class MCP23017 {
public:
  MCP23017(byte address = 0);
  
  void setup(word iomode = 0xFFFF, word pullups = 0x0000);
  
  byte readA();
  byte readB();
  word read();

  void writeA(byte value);
  void writeB(byte value);
  void write(word value);

  void setIOMode(word mode);      // *1 - input, 0 - output
  void setPullup(word mode);      // 1 - enabled, *0 - disabled
  
private:
  byte    _address;
  byte    _status;
  
  byte readRegister(byte addr);
  void writeRegister(byte addr, byte value);
};
