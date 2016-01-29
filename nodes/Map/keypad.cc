#include <Common/util.h>

#include "Keypad.h"

Keypad::Keypad(byte *outPins, byte nOut, byte *inPins, byte nIn, char *keyTable)
  : _outPins(outPins), _nOut(nOut), _inPins(inPins), _nIn(nIn), _keyTable(keyTable)
{
}

void Keypad::setup() {
  for (byte iOut = 0; iOut < _nOut; iOut++) {
    pinMode(_outPins[iOut], OUTPUT);
    digitalWrite(_outPins[iOut], HIGH);
  }
  for (byte iIn = 0; iIn < _nIn; iIn++) {
    pinMode(_inPins[iIn], INPUT);
    digitalWrite(_inPins[iIn], HIGH);
  }
}

word Keypad::scan() {
  word result = 0xFFFF;
  for (byte iOut = 0; iOut < _nOut; iOut++) {
    digitalWrite(_outPins[iOut], LOW);
    for (byte iIn = 0; iIn < _nIn; iIn++) {
      if (digitalRead(_inPins[iIn]) == LOW) {
        result = (iOut << 8) | iIn;
      }
    }
    digitalWrite(_outPins[iOut], HIGH);
  }
  return result;
}
