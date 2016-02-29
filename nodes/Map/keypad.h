#pragma once

#include <Common/types.h>

#define KEY_NONE      0xFFFF

class Keypad {
public:
  //Keypad(byte *outPins, byte nOut, byte *inPins, byte nIn, char *keyTable = 0);  
  Keypad();
  
  void setup();
  
  word scan();
  char getKey();
  
private:
  
  char *_keyTable;
  byte *_outPins;
  byte _nOut;
  byte *_inPins;
  byte _nIn;
};
