#include <Common/util.h>
#include <Common/ws2803s.h>

#define DEFAULT_MINUTES 60
#define DEFAULT_SECONDS 00

#define PIN_SDA   12
#define PIN_CLK   13

class TaskBase {
};

class BombLED {
public:
  BombLED();
  
  void setup();
  void update();

  void reset();
  void complete();

  void tickSecond();
  void toggleBlink();

  void setLeds(word state);
  word getLeds();
    
  void setMinutes(byte minutes);
  byte getMinutes() const;

  void setSeconds(byte seconds);
  byte getSeconds() const;
  
  void setEnabled(byte enabled);
  byte getEnabled() const;
  
  bool isFinished();
  bool isFailed();
    
  //byte callback(byte cmd, byte *params, byte nParams, byte *nResults);
      
private:
  void _setLeds(word indata, byte duty);
  void setDots(byte indata, byte duty);
  void setSegments(byte index, byte mask, byte duty);
  void setDigit(byte index, byte value, byte duty);
  void setDigits(byte mTens, byte mOnes, byte sTens, byte sOnes, byte duty);
  
  WS2803S<PIN_SDA, PIN_CLK, 3>   _driver;
  static const byte _onesmap[];
  static const byte _tensmap[];
  static const byte _ledmap[];
  static const byte _digits[];
  
  bool    _enabled;
  byte    _minutes;
  byte    _seconds;
  bool    _blinkOn;
  bool    _refresh;
  word    _ledState;
};

