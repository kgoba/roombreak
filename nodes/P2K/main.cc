#include <Common/config.h>
#include <Common/serial.h>
#include <Common/modbus.h>
#include <Common/util.h>

#include <util/delay.h>
#include <avr/interrupt.h>

#include "p2k.h"

using namespace P2KConfig;

#define TICK_FREQ       125

#define PIN_OPEN        5

enum {
  FLAG_SECOND,
  FLAG_BLINK
};

enum {
  STATE_INIT,
  STATE_OPEN
};

volatile byte gFlags;
volatile byte gState;

P2K task;

Serial serial;
NewBus bus;
byte busParams[BUS_NPARAMS];

byte busCallback(byte cmd, byte nParams, byte *nResults)
{
  switch (cmd) {
    case CMD_INIT:
    {
      break;      
    }
    
    case CMD_DONE:
    {
      break;      
    }
    
    default:
    break;
  }
  return 0;
}

void setup() {
  gState = STATE_INIT;
  
  pinMode(PIN_OPEN, OUTPUT);
  
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
  static byte led = 0;
  static bool phase = true;
  
  if (bit_check(gFlags, FLAG_BLINK))
  {
    bit_clear(gFlags, FLAG_BLINK);
    
    bool open = false;
    for (byte idx = 0; idx < 5; idx++) {
      byte state = task.getButtons(idx);
      task.setLED(idx, (state & 0x0F) != 0);
      task.setLED(idx + 5, (state & 0xF0) != 0);
      //if ((state & 0x0F) == 0x01) open = true;
    }
    open = task.getButton(0, 0);
    
    if (open && gState == STATE_INIT) {
      pinWrite(PIN_OPEN, HIGH);
      _delay_ms(100);
      pinWrite(PIN_OPEN, LOW);
      gState = STATE_OPEN;
    }
    if (!open && gState == STATE_OPEN) {
      gState = STATE_INIT;
    }
    
    //task.setLED(led, phase);      
    if (++led >= 10) {
      led = 0;
      phase = !phase;
    }
  }
  
  task.update();

  bus.poll();
  _delay_ms(1);
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
  if (millis2 >= 200) {
    millis2 -= 200;
    bit_set(gFlags, FLAG_BLINK);
  }
}
