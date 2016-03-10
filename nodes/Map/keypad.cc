#include <Common/util.h>

#include "keypad.h"

// Keypad parameters
#define N_KEYPAD_COL        4
#define N_KEYPAD_ROW        4

// Keypad pins
#define KEYPAD_COL_PINS     {14, 13, 15, 12}
#define KEYPAD_ROW_PINS     {7, 11, 8, 10}

char keyTable[N_KEYPAD_ROW][N_KEYPAD_COL] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte outPins[] = KEYPAD_COL_PINS;
byte inPins[] = KEYPAD_ROW_PINS;

Keypad::Keypad()
  : _outPins(outPins), _nOut(N_KEYPAD_COL), _inPins(inPins), _nIn(N_KEYPAD_ROW)
{
}

/*
Keypad::Keypad(byte *outPins, byte nOut, byte *inPins, byte nIn, char *keyTable)
  : _outPins(outPins), _nOut(nOut), _inPins(inPins), _nIn(nIn), _keyTable(keyTable)
{
}
*/

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
  word result = KEY_NONE;
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

char Keypad::getKey() {
  word keycode = scan();
  if (keycode == KEY_NONE) return 0;
  byte row = keycode & 0xFF;
  byte col = keycode >> 8;
  return keyTable[row][col];
}
