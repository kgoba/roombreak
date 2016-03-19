#pragma once

#include "util.h"

class IOBus {
public:
  virtual void on(byte index) = 0;
  virtual void off(byte index) = 0;
  virtual void setBit(byte index, bool isOn) {
    if (isOn) on(index);
    else off(index);
  }
  virtual void update() = 0;
};

class PWMBus {
  virtual void set(byte index, byte value) = 0;
  virtual void clear() = 0;
  virtual void update() = 0;
};

template<byte sdaPin, byte clkPin, byte nChips = 1>
class WS2803S : public IOBus, public PWMBus {
public:
  void setup();
  void set(byte index, byte value);
  void on(byte index) { set(index, 255); }
  void off(byte index) { set(index, 0); }
  void clear();

  void update();

private:
  byte  _pwmData[18*nChips];
};

template<byte sdaPin, byte clkPin, byte nChips>
void WS2803S<sdaPin, clkPin, nChips>::setup()
{
  pinMode(sdaPin, OUTPUT);
  pinMode(clkPin, OUTPUT);
  clear();
  update();
}

template<byte sdaPin, byte clkPin, byte nChips>
void WS2803S<sdaPin, clkPin, nChips>::set(byte index, byte value)
{
  if (index < 18*nChips)
    _pwmData[index] = value;
}

template<byte sdaPin, byte clkPin, byte nChips>
void WS2803S<sdaPin, clkPin, nChips>::clear() {
  for (byte i = 0; i < 18*nChips; i++) 
    _pwmData[i] = 0;
}

template<byte sdaPin, byte clkPin, byte nChips>
void WS2803S<sdaPin, clkPin, nChips>::update() {
  for (byte i = 0; i < 18*nChips; i++){
    for (byte j = 0; j < 8; j++)  {
      digitalWrite(sdaPin, (_pwmData[i] & (1 << (7 - j))) ? HIGH : LOW );
      digitalWrite(clkPin, HIGH);
      digitalWrite(clkPin, LOW);        
    }
  }
}
