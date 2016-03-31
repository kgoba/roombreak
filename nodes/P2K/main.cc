#include <Common/config.h>
#include <Common/task.h>

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
  FLAG_BLINK,
  FLAG_OPEN,
  FLAG_CLOSE
};

enum {
  STATE_INIT,
  STATE_OPEN
};

volatile byte gFlags;
volatile byte gState;

P2K panel;
bool gSolved1;
bool gSolved2;
bool gCashOpen;

byte gCount;

void taskRestart() {
  gSolved1 = false;
  gSolved2 = false;
  gCashOpen = false;
  gCount = 0;
  panel.clear();
}

void taskComplete() {
  gSolved1 = true;
  gSolved2 = true;
  gCount++;
  bit_set(gFlags, FLAG_OPEN);
  gCashOpen = true;
}

byte taskIsDone() {
  return gSolved1 && gSolved2;
}

byte taskCallback(byte cmd, byte nParams, byte *nResults, byte *busParams)
{
  switch (cmd) {
    case CMD_COUNT:
    {
      *nResults = 1;
      busParams[0] = gCount;
    }
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

  taskRestart();
  taskSetup(BUS_ADDRESS);
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
  
    bool wasDone = taskIsDone();
      if (!gSolved1 && ok1) {
        for (byte idx = 0; idx < 5; idx++) panel.setLED(idx, true);
        gSolved1 = ok1;
      }
      if (!gSolved2 && ok2) {
        for (byte idx = 0; idx < 5; idx++) panel.setLED(idx + 5, true);
        gSolved2 = ok2;
      }
      if (gSolved1 && !ok1) {
        for (byte idx = 0; idx < 5; idx++) panel.setLED(idx, false);
        gSolved1 = ok1;
      }
      if (gSolved2 && !ok2) {
        for (byte idx = 0; idx < 5; idx++) panel.setLED(idx + 5, false);
        gSolved2 = ok2;
      }
      if (taskIsDone()) {
        
        if (!wasDone) {
          led = 0; 
          phase = true; 
          taskComplete();
          
        }

        panel.setLED(led, phase);            
        if (++led >= 10) {
          led = 0;
          phase = !phase;
        }
      }
    
  }

  taskLoop();
}

ISR(TIMER0_OVF_vect) {
  static word millis = 0;
  static word millis2 = 0;
  static word millis3 = 0;
  static word millis4 = 0;
  
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

  if (bit_check(gFlags, FLAG_OPEN)) {
    millis3 += (1000UL / TICK_FREQ);
    
    if (millis3 > 500) {
      bit_clear(gFlags, FLAG_OPEN);
      pinWrite(PIN_OPEN, HIGH);
      millis4 = 0;
      bit_set(gFlags, FLAG_CLOSE);
    }
  }
  if (bit_check(gFlags, FLAG_CLOSE)) {
    millis4 += (1000UL / TICK_FREQ);
    
    if (millis4 > 100) {
      bit_clear(gFlags, FLAG_CLOSE);
      pinWrite(PIN_OPEN, LOW);       
      millis3 = 0;
    }
  }
}
