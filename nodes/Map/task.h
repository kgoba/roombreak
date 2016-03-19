#pragma once

#include <Common/types.h>
#include <Common/serial.h>

#include "config.h"

#define N_BUFFER    6
#define N_TICKETS   4

// Task configuration
struct Config {
  struct TicketInfo {
    byte ledMap[5];
    char number[N_BUFFER];
  
    void init(byte *ledMap, char *number) {
      for (byte i = 0; i < 5; i++) this->ledMap[i] = ledMap[i];
      for (byte i = 0; i < N_BUFFER; i++) this->number[i] = number[i];
    }
  };
  
  void save();
  void load();
  
  word getRegister(byte address);
  
  TicketInfo tickets[N_TICKETS];
};

class Task {
public:
  void setup();
  void loop();
  void tick();
  
  void restart();
  void complete();
  bool isFinished();
  
  void loadSettings();
  void saveSettings();
    
private:
  Config _config;
  
  enum { 
    ENTERING,
    ENTERED
  } _state;
  
  byte _iEntered;
  char _keyBuffer[N_BUFFER];
  byte _keyBufferSize;
  byte _ledMap[5];
  char _lastKey;
    
  byte processKey(char key);
  void showRoute(byte iTicket);
};
