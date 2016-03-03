#pragma once

#include "util.h"
#include "ws2803s.h"

class AudioPlayer {
public:
  AudioPlayer(WS2803S &expander, byte pin0, byte pin1, byte pin2) 
    : _expander(expander), _pin0(pin0), _pin1(pin1), _pin2(pin2) {}
  
  void setup();
  void play(byte id);
private:
  byte _pin0, _pin1, _pin2;
  WS2803S &_expander;
};
