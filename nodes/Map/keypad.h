#pragma once

#include <Common/types.h>

class Keypad {
public:
  Keypad(byte *outPins, byte nOut, byte *inPins, byte nIn, char *keyTable = 0);  
  
  void setup();
  
  word scan();
private:
  
  char *_keyTable;
  byte *_outPins;
  byte _nOut;
  byte *_inPins;
  byte _nIn;
};
