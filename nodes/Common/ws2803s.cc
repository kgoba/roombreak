#include "ws2803s.h"

#include <stdlib.h>

WS2803S::WS2803S(byte sdaPin, byte clkPin, byte nChips)
{
  _sdaPin = sdaPin;
  _clkPin = clkPin;
  _nPins = 18 * nChips;
  _pwmData = (byte *)malloc(_nPins * sizeof(byte));
}

WS2803S::~WS2803S()
{
  free((void *)_pwmData);
}

void WS2803S::setup()
{
  pinMode(_sdaPin, OUTPUT);
  pinMode(_clkPin, OUTPUT);
  clear();
  update();
}

void WS2803S::set(byte index, byte value)
{
  if (index < _nPins)
    _pwmData[index] = value;
}

void WS2803S::clear() {
  for (byte i = 0; i < _nPins; i++) 
    _pwmData[i] = 0;
}

void WS2803S::update() {
  for (byte i = 0; i < _nPins; i++){
    for (byte j = 0; j < 8; j++)  {
      digitalWrite(_sdaPin, (_pwmData[i] & (1 << (7 - j))) ? HIGH : LOW );
      digitalWrite(_clkPin, HIGH);
      digitalWrite(_clkPin, LOW);        
    }
  }
}
