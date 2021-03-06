#include <Common/util.h>
#include <Common/mcp23017.h>

#define N_ROWS          5

class P2K {
public:
    
  static void setup();
  static void update();
  static void clear();
  
  static void enableLED(byte idx);
  static void disableLED(byte idx);
  static void setLED(byte idx, bool enabled);
  
  static byte getButtons(byte idx);
  static bool getButton(byte row, byte column);
    
private:
  static MCP23017    _driver;
  static byte        _buttons[N_ROWS];
  static byte        _leds[N_ROWS];
  static byte        _scanRow;
};
