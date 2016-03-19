#include "audioplayer.h"

void AudioPlayer::play(byte id) {
  if (_pin0) _bus.setBit(_pin0, (id & 0b00001));
  if (_pin1) _bus.setBit(_pin1, (id & 0b00010));
  if (_pin2) _bus.setBit(_pin2, (id & 0b00100));
  if (_pin3) _bus.setBit(_pin3, (id & 0b01000));
  if (_pin4) _bus.setBit(_pin4, (id & 0b10000));
  _bus.update();
}

void AudioPlayer::setup() {
  stop();
}

void AudioPlayer::stop() {
  play(0);
}
