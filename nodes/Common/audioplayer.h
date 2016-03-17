#pragma once

#include "util.h"
#include "ws2803s.h"

class AudioPlayer {
public:
  AudioPlayer(WS2803S &expander, byte pin0, byte pin1 = 0, byte pin2 = 0, byte pin3 = 0, byte pin4 = 0) 
    : _expander(expander), _pin0(pin0), _pin1(pin1), _pin2(pin2), _pin3(pin3), _pin4(pin4) {}
  
  void setup();
  void play(byte id);
  void stop();
  
private:
  byte _pin0, _pin1, _pin2, _pin3, _pin4;
  WS2803S &_expander;
};

class AudioShield {
  
public:
  AudioPlayer *player1;
  AudioPlayer *player2;
  AudioPlayer *player3;
  WS2803S _expander;
};
