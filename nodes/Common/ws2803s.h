#pragma once

#include "util.h"

class WS2803S {
public:
  WS2803S(byte sdaPin, byte clkPin, byte nChips = 1);
  ~WS2803S();

  void setup();
  void set(byte index, byte value);
  void clear();

  void update();

private:
  byte  _sdaPin;
  byte  _clkPin;
  byte  _nPins;
  
  byte  *_pwmData;
};
