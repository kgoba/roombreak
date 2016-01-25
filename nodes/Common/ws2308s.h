#include "util.h"

class WS2308S {
public:
  WS2308S(byte sdaPin, byte clkPin, byte nChips = 1);
  ~WS2308S();

  void setup();
  void set(byte index, byte value);

  void update();

private:
  byte  _sdaPin;
  byte  _clkPin;
  byte  _nPins;
  
  byte  *_pwmData;
};
