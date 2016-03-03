#include "audioplayer.h"

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
