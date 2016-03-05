#include "line.h"

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
  pinWrite(_config.pinRing, _ringing ? HIGH : LOW);
}

void PLine::update() {
  // read analog value and update the moving average
  word aval = adcRead(_config.apinSense);
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
