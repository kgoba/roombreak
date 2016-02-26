#include <Common/util.h>
#include <Common/serial.h>
#include <Common/modbus.h>
#include <Common/config.h>

#include <util/delay.h>
#include <avr/interrupt.h>

#include "bombled.h"

#define BUS_ADDRESS     ADDRESS_BOMB
#define BUS_NPARAMS     4

#define TICK_FREQ       125

enum {
  FLAG_SECOND,
  FLAG_BLINK
};

volatile byte gFlags;

BombLED task;

Serial serial;
NewBus bus;
byte busParams[BUS_NPARAMS];

byte busCallback(byte cmd, byte nParams, byte *nResults)
{
  //return task.callback(cmd, busParams, nParams, nResults);
  switch (cmd) {
    case 0x01: {
      if (nParams > 0) {
        task.setMinutes(busParams[0]);
      }
      *nResults = 1;
      busParams[0] = task.getMinutes();
      break;
    }
    case 0x02: {
      if (nParams > 0) {
        task.setSeconds(busParams[0]);
      }
      *nResults = 1;
      busParams[0] = task.getSeconds();
      break;
    }
    case 0x03: {
      *nResults = 1;
      busParams[0] = task.isFinished();
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
  serial.setup(BUS_SPEED, PIN_TXE, PIN_RXD);
  serial.enable();  
  bus.setup(BUS_ADDRESS, &busCallback, busParams, BUS_NPARAMS);
}

void loop() {
  if (bit_check(gFlags, FLAG_SECOND)) {
    bit_clear(gFlags, FLAG_SECOND);
    task.tickSecond();
  }
  if (bit_check(gFlags, FLAG_BLINK)) {
    bit_clear(gFlags, FLAG_BLINK);
    task.toggleBlink();
  }
  task.update();

  bus.poll();
  _delay_ms(100);
}

ISR(TIMER0_OVF_vect) {
  static word millis = 0;
  static word millis2 = 0;
  
  millis += (1000UL / TICK_FREQ);
  millis2 += (1000UL / TICK_FREQ);
  
  if (millis >= 1000) {
    millis -= 1000;
    bit_set(gFlags, FLAG_SECOND);
  }
  if (millis2 >= 500) {
    millis2 -= 500;
    bit_set(gFlags, FLAG_BLINK);
  }
}
