#pragma once

#include <Common/types.h>
#include <Common/serial.h>

#include "config.h"

class Task {
public:
  void setup();
  void loop();
  
  void loadSettings();
  void saveSettings();
    
private:
  Config _config;
};
