#include "line.h"

void AudioPlayer::play(byte id) {
  _expander.set(_pin0, (id & 0b001) ? 0xFF : 0);
  _expander.set(_pin1, (id & 0b010) ? 0xFF : 0);
  _expander.set(_pin2, (id & 0b100) ? 0xFF : 0);
  _expander.update();
  //digitalWrite(_pin0, (id & 0b001) ? LOW : HIGH);
  //digitalWrite(_pin1, (id & 0b010) ? LOW : HIGH);
  //digitalWrite(_pin2, (id & 0b100) ? LOW : HIGH);
}

void AudioPlayer::setup() {
  //pinMode(_pin0, OUTPUT);
  //pinMode(_pin1, OUTPUT);
  //pinMode(_pin2, OUTPUT);
  play(0);
}

void PLine::setup() {
  pinMode(_config.pinRing, OUTPUT);
}

void PLine::setTone(ToneType type) {
  switch (type) {
    case TONE_OFF:
      _player.play(0);
      break;
    case TONE_DIAL:
      _player.play(_config.trackDial);
      break;
    case TONE_CALL:
      _player.play(_config.trackCall);
      break;    
    case TONE_BUSY:
      _player.play(_config.trackBusy);
      break;
  }
}

void PLine::playCustom(byte id) {
  _player.play(id);
}

void PLine::setRing(bool ringing) {
  _ringing = ringing;
  digitalWrite(_config.pinRing, _ringing ? HIGH : LOW);
}

void PLine::update() {
  // read analog value and update the moving average
  word aval = analogRead(_config.apinSense);
  _senseAvg += aval - (_senseAvg >> 4);
  
  // calculate 6.10-bit fixed point value
  word avg = (_senseAvg >> 4);
  
  if (_state == OPEN && avg > VOLTS_TO_COUNTS(THRESHOLD_CLOSED))
    _state = CLOSED;
  else if (_state == CLOSED && avg < VOLTS_TO_COUNTS(THRESHOLD_OPEN))
    _state = OPEN;
}

PLine::State PLine::getState() {
  return _state;
}
