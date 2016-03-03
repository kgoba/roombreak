#pragma once

#include "config.h"
#include <Common/util.h>
#include <Common/ws2803s.h>
#include <Common/audioplayer.h>

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

