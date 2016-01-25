#include <Common/util.h>

#include <util/delay.h>

#include "bombled.h"

class BombTask {
public:
  BombTask();  
  void setup();
  
  void update();
  void minusSecond();
    
private:
  BombLED _display;

  byte    _minutes;
  byte    _seconds;
};

BombTask::BombTask()
{
  _minutes = 8;
  _seconds = 48;
}

void BombTask::setup()
{
  _display.setup();
}

void BombTask::update()
{
  _display.setDigits(_minutes / 10, _minutes % 10, _seconds / 10, _seconds % 10, 255);
  _display.update();
}

void BombTask::minusSecond()
{
  if (_seconds > 0) {
    _seconds--;
    return;
  }
  if (_minutes > 0) {
    _seconds = 59;
    _minutes--;
  }
}


BombTask task;

void setup() {
  task.setup();
  _delay_ms(1000);
}

void loop() {
  task.update();
  task.minusSecond();
  _delay_ms(100);
}

int main()
{
  setup();
  
  while (true) {
    loop();
  }
  return 0;
}

