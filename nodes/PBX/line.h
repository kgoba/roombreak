#pragma once

#include "config.h"
#include <Common/util.h>
#include <Common/ws2308s.h>


class AudioPlayer {
public:
  AudioPlayer(WS2308S &expander, byte pin0, byte pin1, byte pin2) 
    : _expander(expander), _pin0(pin0), _pin1(pin1), _pin2(pin2) {}
  
  void setup();
  void play(byte id);
private:
  byte _pin0, _pin1, _pin2;
  WS2308S &_expander;
};


struct PLineConfig
{
  byte apinSense;
  byte pinRing;
  byte trackDial;
  byte trackCall;
  byte trackBusy;
};


class PLine {
public:
  PLine(AudioPlayer &player, const PLineConfig &config) 
    : _player(player), _config(config), _state(OPEN)
  {}
  
  enum State {
    OPEN, CLOSED, SHORT
  };

  enum ToneType {
    TONE_OFF, TONE_DIAL, TONE_CALL, TONE_BUSY
  };

  void setup();
  void setTone(ToneType type);
  void playCustom(byte id);
  void setRing(bool ringing);
  State getState();
  void update();
  
private:
  bool        _ringing;
  AudioPlayer _player;
  PLineConfig _config;
  word        _senseAvg;
  State       _state;
};

