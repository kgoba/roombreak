#pragma once

#include <Common/types.h>
#include <Common/serial.h>

#include "config.h"

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

class Task {
public:
  void setup();
  void loop();
  
  void loadSettings();
  void saveSettings();
    
private:
  Config _config;
  
  enum { 
    ENTERING,
    ENTERED
  } _state;
  
  byte _iEntered;
  
  void processKey(char key);
};
