#pragma once

#include <Common/types.h>

// Keypad parameters
#define N_KEYPAD_COL        4
#define N_KEYPAD_ROW        4
// Keypad pins
#define KEYPAD_COL_PINS     {14, 13, 15, 12}
#define KEYPAD_ROW_PINS     {7, 11, 8, 10}
// LED driver pins
#define LED_SDA             19
#define LED_CLK             18

// Task configuration
struct Config {
  struct TicketInfo {
    byte ledMap[5];
    char number[4];
  
    void init(byte *ledMap, char *number) {
      for (byte i = 0; i < 5; i++) this->ledMap[i] = ledMap[i];
      for (byte i = 0; i < 4; i++) this->number[i] = number[i];
    }
  };
  
  void save();
  void load();
  
  word getRegister(byte address);
  
  TicketInfo tickets[4];
};
