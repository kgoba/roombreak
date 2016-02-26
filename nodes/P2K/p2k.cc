#include "p2k.h"
#include <util/delay.h>

// I2C subaddress (A0-A2 pins)
#define SUBADDRESS      0

// pin bit numbers at MCP chip (0 - PA0, 7 - PB7, 8 - PB0, 15 - PB7)
#define PIN_LED1        (8+7)
#define PIN_LED2        (8+6)
#define PIN_J1          (8+5)
#define PIN_J2          (8+4)
#define PIN_J3          (8+3)
#define PIN_J4          (8+2)
#define PIN_J5          (8+1)
#define PIN_D5_2        7
#define PIN_D4_2        6
#define PIN_D3_2        5
#define PIN_D2_2        4
#define PIN_D5_1        3
#define PIN_D4_1        2
#define PIN_D3_1        1
#define PIN_D2_1        0

//#define IODIR_MASK      0b0000000111111111
#define IODIR_MASK      0x00FF

//static byte pollOrder[N_ROWS] = { PIN_J1, PIN_J2, PIN_J3, PIN_J4, PIN_J5 };

MCP23017    P2K::_driver(SUBADDRESS);
byte        P2K::_buttons[N_ROWS];
byte        P2K::_leds[N_ROWS];
byte        P2K::_scanRow;

void P2K::setup()
{
  _driver.setup(IODIR_MASK, IODIR_MASK);
  
  for (byte i = 0; i < N_ROWS; i++) {
    _buttons[i] = 0;
    _leds[i] = 0;
  }
  _scanRow = 0;
}

void P2K::update()
{
  byte outB = 0b00111110 | (_leds[_scanRow] & 0xC0);
  bit_clear(outB, _scanRow + 1);
  _driver.writeB(outB);
  byte inA = _driver.readA();
  _buttons[_scanRow] = ~inA;

  if (++_scanRow >= N_ROWS) _scanRow = 0;
}

void P2K::clear()
{
  _driver.writeB(0b00111110);
}

void P2K::enableLED(byte index) 
{
  bit_set(_leds[index % N_ROWS], 7 - (index / N_ROWS));
}

void P2K::disableLED(byte index)
{
  bit_clear(_leds[index % N_ROWS], 7 - (index / N_ROWS));
}

void P2K::setLED(byte index, bool enabled)
{
  if (enabled) enableLED(index);
  else disableLED(index);
}

byte P2K::getButtons(byte idx)
{
  return _buttons[idx];
}