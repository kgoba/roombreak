#include "config.h"
#include "serial.h"
#include "modbus.h"
#include "util.h"
#include "task.h"

#include <util/delay.h>
#include <avr/interrupt.h>

static Serial serial;
static NewBus bus;
static byte busParams[BUS_NPARAMS];

static bool gTaskEnabled;

bool taskIsEnabled() {
  return gTaskEnabled;
}

static byte busCallback(byte cmd, byte nParams, byte *nResults)
{
  byte result = 0;
  switch (cmd) {
    case CMD_ENABLED:
    {
      if (nParams > 0) {
        gTaskEnabled = busParams[0];
      }
      *nResults = 1;
      busParams[0] = gTaskEnabled;
      break;      
    }
    
    case CMD_DONE:
    {
      if (nParams > 0) {
        byte done = busParams[0];
        if (done) taskComplete();
        else taskRestart();
      }
      *nResults = 1;
      busParams[0] = taskIsDone();
      break;      
    }
        
    default:
      result = taskCallback(cmd, nParams, nResults, busParams);
      break;
  }
  return result;
}

void taskSetup(byte address) {  
  serial.setup(BUS_SPEED, PIN_TXE, PIN_RXD);
  serial.enable();  
  bus.setup(address, &busCallback, busParams, BUS_NPARAMS);
  gTaskEnabled = true;
}

void taskLoop() {
  //taskLoop();

  bus.poll();
  _delay_ms(LOOP_DELAY_MS);
}

/*
Serial gSerial;
NewBus gBus;
byte gBusParams[BUS_NPARAMS];
TaskBase *gTask;

byte busCallback2(byte cmd, byte nParams, byte *nResults)
{
  byte result = 0;
  switch (cmd) {
    case CMD_INIT:
    {
      //gTaskDone = 0;
      //nResults = 1;
      //busParams[0] = gInitDone;
      break;      
    }
    
    case CMD_DONE:
    {
      if (nParams > 0) {
        byte done = gBusParams[0];
        if (done) gTask->complete();
        else gTask->restart();
      }
      *nResults = 1;
      gBusParams[0] = gTask->isDone();
      break;      
    }
        
    default:
      result = gTask->callback(cmd, nParams, nResults, gBusParams);
      break;
  }
  return result;
}

void taskSetup(TaskBase *task, byte address) {
  gTask = task;
  
  gTask->setup();

  gSerial.setup(BUS_SPEED, PIN_TXE, PIN_RXD);
  gSerial.enable();  
  gBus.setup(address, &busCallback, gBusParams, BUS_NPARAMS);
}

void loop() {
  gTask->loop();

  gBus.poll();
  _delay_ms(LOOP_DELAY_MS);
}

void taskRegister(TaskBase *task, byte address) {
  gTask = task;
  
  gTask->setup();

  gSerial.setup(BUS_SPEED, PIN_TXE, PIN_RXD);
  gSerial.enable();  
  gBus.setup(address, &busCallback2, gBusParams, BUS_NPARAMS);
}

void TaskBase::setup(byte address) {
  serial.setup(BUS_SPEED, PIN_TXE, PIN_RXD);
  serial.enable();  
  bus.setup(address, &busCallback2, busParams, BUS_NPARAMS);
}

void TaskBase::loop() {
  bus.poll();
  _delay_ms(LOOP_DELAY_MS);
}
*/