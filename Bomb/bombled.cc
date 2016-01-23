#include "bombled.h"

byte sdaPin = 12;
byte clkPin = 13;

const byte BombLED::_onesmap[]={16,17,0,1,2,14,13,15};
const byte BombLED::_tensmap[]={11,12,5,6,7,9,8,10};
const byte BombLED::_ledmap[]={17,15,14,13,12,11,10,9,8,7,1,16};


const byte BombLED::_digits[11]={
  0b01111110,     // 0
  0b00110000,     // 1
  0b01101101,
  0b01111001,
  0b00110011,     // 4
  0b01011011,
  0b01011111,
  0b01110000,     // 7
  0b01111111,     // 8
  0b01111011,     // 9
  0b00000000      // OFF
};

BombLED::BombLED() : _driver(sdaPin, clkPin, 3)
{
  
}

void BombLED::setup()
{
  _driver.setup();
  update();
}

void BombLED::update()
{
  _driver.update();
}

void BombLED::setLeds(unsigned int indata, byte duty) {
  for (byte i=0;i<12;i++){
    _driver.set(_ledmap[i]+18, (indata & (1 << i))?duty : 0);
  }
}

void BombLED::setDots(byte indata, byte duty) {
  _driver.set(_tensmap[7], (indata & (1 << 3))?duty : 0); //mTens
  _driver.set(_onesmap[7], (indata & (1 << 2))?duty : 0); //mOnes
  _driver.set(_tensmap[7]+36, (indata & (1 << 1))?duty : 0); //sTens
  _driver.set(_onesmap[7]+36, (indata & (1 << 0))?duty : 0); //sOnes
}

void BombLED::setDigit(byte index, byte value, byte duty) {
  setSegments(index, _digits[value], duty);
}

void BombLED::setSegments(byte index, byte mask, byte duty) {
  for (byte i = 0; i < 7; i++) {
    byte bit_idx;
    switch (index) {
    case 0:
      bit_idx = _tensmap[i];
      break;
    case 1:
      bit_idx = _onesmap[i];
      break; 
    case 2:
      bit_idx = _tensmap[i]+36;
      break;
    case 3:
      bit_idx = _onesmap[i]+36;
      break;
    }
    _driver.set(bit_idx, (mask & (1 << (6-i)))?duty : 0);
  }
}

void BombLED::setDigits(byte mTens,byte mOnes,byte sTens,byte sOnes, byte duty) {
  byte i;
  for (i=0;i<7;i++){
    _driver.set(_tensmap[i], (_digits[mTens] & (1 << (6-i)))?duty : 0);
    _driver.set(_onesmap[i], (_digits[mOnes] & (1 << (6-i)))?duty : 0);
    _driver.set(_tensmap[i]+36, (_digits[sTens] & (1 << (6-i)))?duty : 0);
    _driver.set(_onesmap[i]+36, (_digits[sOnes] & (1 << (6-i)))?duty : 0);
  }
}
