#pragma once

#include "util.h"


class AudioPlayer {
public:
  AudioPlayer(byte pin0, byte pin1, byte pin2) 
    : _pin0(pin0), _pin1(pin1), _pin2(pin2) {}
  
  void setup();
  void play(byte id);
private:
  byte _pin0, _pin1, _pin2;
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
  PLine(AudioPlayer &player, const PLineConfig &config) : _player(player), _config(config) 
  {}
  
  enum State {
    OPEN, CLOSED, SHORT
  };

  enum ToneType {
    TONE_OFF, TONE_DIAL, TONE_CALL, TONE_BUSY
  };

  void setTone(ToneType type);
  void setRing(bool ringing);
  State getState();
  
private:
  bool        _ringing;
  AudioPlayer _player;
  PLineConfig _config;
  word        _senseAvg;
};

