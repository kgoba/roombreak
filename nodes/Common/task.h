#include "config.h"
#include "serial.h"
#include "modbus.h"
#include "util.h"

void taskSetup(byte address);
void taskLoop();

void taskRestart();
void taskComplete();
byte taskIsDone();
byte taskCallback(byte cmd, byte nParams, byte *nResults, byte *busParams);

void setup();
void loop();

/*
void taskSetup(TaskBase *task, byte address);

class TaskBase {
public:
  virtual void restart() = 0;
  virtual void complete() = 0;
  virtual byte isDone() = 0;
  virtual byte callBack(byte cmd, byte nIn, byte *nOut, byte *busParams) = 0;
  
private:
  
};
*/