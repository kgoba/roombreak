#include <Common/util.h>
#include <Common/ws2308s.h>

class BombLED {
public:
  BombLED();
  
  void setup();
  
  void setLeds(unsigned int indata, byte duty);
  void setDots(byte indata, byte duty);
  void setSegments(byte index, byte mask, byte duty);
  void setDigit(byte index, byte value, byte duty);
  void setDigits(byte mTens, byte mOnes, byte sTens, byte sOnes, byte duty);
  
  void update();

private:
  WS2308S   _driver;
  static const byte _onesmap[];
  static const byte _tensmap[];
  static const byte _ledmap[];
  static const byte _digits[];
};

