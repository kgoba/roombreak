#include "line.h"

void AudioPlayer::play(byte id) {
  digitalWrite(_pin0, (id & 0b001) ? LOW : HIGH);
  digitalWrite(_pin1, (id & 0b010) ? LOW : HIGH);
  digitalWrite(_pin2, (id & 0b100) ? LOW : HIGH);
}

void AudioPlayer::setup() {
  pinMode(_pin0, OUTPUT);
  pinMode(_pin1, OUTPUT);
  pinMode(_pin2, OUTPUT);
}

void PLine::setTone(ToneType type)
{
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

PLine::State PLine::getState()
{
  // read analog value and update the moving average
  word aval = analogRead(_config.apinSense);
  _senseAvg += aval - (_senseAvg >> 6);
  
  // calculate 6.10-bit fixed point value
  word avg = (_senseAvg >> 6);
  
  // compare to thresholds
  if (avg < VOLTS_TO_COUNTS(THRESHOLD_OPEN))
    return OPEN;
  if (avg > VOLTS_TO_COUNTS(THRESHOLD_CLOSED))
    return CLOSED;
}
