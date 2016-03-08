#include "audioplayer.h"

void AudioPlayer::play(byte id) {
  if (_pin0) _expander.set(_pin0, (id & 0b00001) ? 0xFF : 0);
  if (_pin1) _expander.set(_pin1, (id & 0b00010) ? 0xFF : 0);
  if (_pin2) _expander.set(_pin2, (id & 0b00100) ? 0xFF : 0);
  if (_pin3) _expander.set(_pin3, (id & 0b01000) ? 0xFF : 0);
  if (_pin4) _expander.set(_pin4, (id & 0b10000) ? 0xFF : 0);
  _expander.update();
}

void AudioPlayer::setup() {
  stop();
}

void AudioPlayer::stop() {
  play(0);
}