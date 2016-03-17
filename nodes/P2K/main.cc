#include <Common/config.h>
#include <Common/serial.h>
#include <Common/modbus.h>
#include <Common/util.h>

#include <util/delay.h>
#include <avr/interrupt.h>

#include "p2k.h"

using namespace P2KConfig;

#define TICK_FREQ       250

#define PIN_OPEN        5

const byte SOLUTION1[5] = { 3, 2, 1, 4, 4 };
const byte SOLUTION2[5] = { 3, 2, 2, 2, 1 };

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

P2K panel;
byte gTaskDone;

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
  TIMER0_SETUP(TIMER0_FAST_PWM, TIMER0_PRESCALER(TICK_FREQ));
  //TCCR0A = (1 << WGM01) | (1 << WGM00);
  //TCCR0B = (1 << CS02) | (1 << CS00) | (1 << WGM02);
  TIMSK0 = (1 << TOIE0); 
  //OCR0A = (byte)(F_CPU / (1024UL * TICK_FREQ)) - 1;
  OCR0A = TIMER0_COUNTS(TICK_FREQ) - 1;

  panel.setup();
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
    
    bool ok1 = true;
    bool ok2 = true;
    for (byte idx = 0; idx < 5; idx++) {
      byte state = panel.getButtons(idx);
      byte state1 = state & 0x0F;
      byte state2 = state & 0xF0;
      byte mask1 = 1 << (4 - SOLUTION1[idx]);
      byte mask2 = 1 << (8 - SOLUTION2[idx]);
      if (state1 != mask1) {
        ok1 = false;
      }
      if (state2 != mask2) {
        ok2 = false;
      }
      //if (!gTaskDone) {
      //  panel.setLED(idx, (state1 == mask1));
      //  panel.setLED(idx + 5, (state2 == mask2));
      //}
    }
    
    if (!gTaskDone && ok1) {
      for (byte idx = 0; idx < 5; idx++) panel.setLED(idx, true);
    }
    if (!gTaskDone && ok2) {
      for (byte idx = 0; idx < 5; idx++) panel.setLED(idx + 5, true);
    }
    //bool open = panel.getButton(0, 0);    
    bool open = ok1 & ok2;
    if (open) {
      if (!gTaskDone) {
        gTaskDone = true;
        led = 0; 
        phase = true; 
        pinWrite(PIN_OPEN, HIGH);
        _delay_ms(100);
        pinWrite(PIN_OPEN, LOW);        
      }
    }

    if (gTaskDone) {
      panel.setLED(led, phase);            
    }
    if (++led >= 10) {
      led = 0;
      phase = !phase;
    }
  }
  
  bus.poll();
  _delay_ms(20);
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
  
  panel.update();
}
