#include <Common/util.h>
#include <Common/ws2803s.h>

#define DEFAULT_MINUTES 60
#define DEFAULT_SECONDS 00

class BombLED {
public:
  BombLED();
  
  void setup();
  void clear();
  void update();

  void tickSecond();
  void toggleBlink();
    
  void setMinutes(byte minutes);
  byte getMinutes() const;

  void setSeconds(byte seconds);
  byte getSeconds() const;
  
  bool isFinished();
    
  //byte callback(byte cmd, byte *params, byte nParams, byte *nResults);
      
private:
  void setLeds(unsigned int indata, byte duty);
  void setDots(byte indata, byte duty);
  void setSegments(byte index, byte mask, byte duty);
  void setDigit(byte index, byte value, byte duty);
  void setDigits(byte mTens, byte mOnes, byte sTens, byte sOnes, byte duty);
  
  WS2803S   _driver;
  static const byte _onesmap[];
  static const byte _tensmap[];
  static const byte _ledmap[];
  static const byte _digits[];
  
  byte    _minutes;
  byte    _seconds;
  bool    _blinkOn;
  bool    _refresh;
};

