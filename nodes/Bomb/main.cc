#include <Common/config.h>
#include <Common/task.h>

#include <util/delay.h>
#include <avr/interrupt.h>

#include "bombled.h"

using namespace BombConfig;

#define TICK_FREQ       125

enum {
  FLAG_SECOND,
  FLAG_BLINK
};

volatile byte gFlags;

BombLED task;

void taskComplete() {
  task.complete();
}

void taskRestart() {
  task.reset();
}

byte taskIsDone() {
  return task.isFinished();
}

byte taskCallback(byte cmd, byte nParams, byte *nResults, byte *busParams)
{
  //return task.callback(cmd, busParams, nParams, nResults);
  switch (cmd) {
    case CMD_TIME: {
      if (nParams > 1) {
        task.setMinutes(busParams[0]);
        task.setSeconds(busParams[1]);
      }
      *nResults = 2;
      busParams[0] = task.getMinutes();
      busParams[1] = task.getSeconds();
      break;
    }
    case CMD_LEDS: {
      if (nParams > 1) {
        task.setLeds(WORD_LH(busParams[0], busParams[1]));
      }
      *nResults = 2;
      word leds = task.getLeds();
      busParams[0] = WORD_LO(leds);
      busParams[1] = WORD_HI(leds);
      break;
    }
    case CMD_ENABLE: {
      if (nParams > 0) {
        task.setEnabled(busParams[0]);
      }
      *nResults = 1;
      busParams[0] = task.getEnabled();
      break;
    }
  }
  return 0;
}

void setup() {
  // Setup Timer0
  // Set CTC mode, TOP = OCRA, prescaler 1024
  // Overflow 125Hz (8ms)
  TCCR0A = (1 << WGM01) | (1 << WGM00);
  TCCR0B = (1 << CS02) | (1 << CS00) | (1 << WGM02);
  TIMSK0 = (1 << TOIE0); 
  OCR0A = (byte)(F_CPU / (1024UL * TICK_FREQ)) - 1;

  task.setup();
  taskSetup(BUS_ADDRESS);
  taskRestart();
}

void loop() {
  if (bit_check(gFlags, FLAG_SECOND)) {
    bit_clear(gFlags, FLAG_SECOND);
    //task.tickSecond();
  }
  if (bit_check(gFlags, FLAG_BLINK)) {
    bit_clear(gFlags, FLAG_BLINK);
    //task.toggleBlink();
  }
  //task.update();

  taskLoop();
}

ISR(TIMER0_OVF_vect) {
  static word millis = 0;
  static word millis2 = 0;
  
  millis += (1000UL / TICK_FREQ);
  millis2 += (1000UL / TICK_FREQ);
  
  if (millis >= 1000) {
    millis -= 1000;
    bit_set(gFlags, FLAG_SECOND);
    task.tickSecond();
  }
  if (millis2 >= 500) {
    millis2 -= 500;
    bit_set(gFlags, FLAG_BLINK);
    task.toggleBlink();
    task.update();
  }
}
