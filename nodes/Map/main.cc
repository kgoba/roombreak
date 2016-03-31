#include <Common/config.h>
#include <Common/task.h>
#include <Common/pins.h>

#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>

#include "task.h"

using namespace MapConfig;

// Timer tick frequency in Hz (exact: 125, 250, 500, 1000) (16000000 = 5*5*5 * 5*5*5 * 16)
#define TICK_FREQ   125

#define PIN_BUZZER  5 // PWM
/*
Uzdevums:

Ievadīt 4 biļetes pareizā secībā. 
Biļetes nr. ir 6 simboli, ko ievada ar keypad. Nospiežot pogu, ir skaņas indikācija.
Pēc katras pareizās biļetes ievadīšanas atskan skaņas indikācija un iedegas noteikti LED.
Ievadot jebko nepareizu, atgriežas sākuma stāvoklī.
Pēc 4 pareizu biļešu ievadīšanas atskan skaņas indikācija (globāla?) un uzdevums ir atrisināts.

3 skaņas indikācijas:
1) poga nospiesta
2) biļetes numurs ievadīts
3) uzdevums atrisināts

Uzdevuma konfigurācija:
- 1. biļetes maršruta LED saraksts (5 baiti)
- 2. biļetes maršruta LED saraksts (5 baiti)
- 3. biļetes maršruta LED saraksts (5 baiti)
- 4. biļetes maršruta LED saraksts (5 baiti)
- 1. biļetes numurs (4 baiti)
- 2. biļetes numurs (4 baiti)
- 3. biļetes numurs (4 baiti)
- 4. biļetes numurs (4 baiti)

Hardware:
- Keypad (4x4)
- 1 izeja buzzerim
- 2xWS2803 LED (36 gab.)
 */

Task task;

volatile byte gFlags;
volatile word gMillis;

void taskComplete() {
  task.complete();
}

void taskRestart() {
  task.restart();
}

byte taskIsDone() {
  return task.isFinished();
}

byte taskCallback(byte cmd, byte nParams, byte *nResults, byte *busParams)
{
  switch (cmd) {
  }
  return 0;
}


void setup() {  
  // Setup Timer0
  // Set CTC mode, TOP = OCRA, prescaler 256 (4us)
  //TCCR0A = (1 << WGM01);
  //TCCR0B = (1 << CS02);

  // Setup Timer2
  // Set CTC mode, TOP = OCRA, prescaler 1024
  // Overflow 125Hz (8ms), overflow interrupt enabled
  TIMER2_SETUP(TIMER2_CTC, TIMER2_PRESCALER(TICK_FREQ));
  OCR2A = TIMER2_COUNTS(TICK_FREQ) - 1;
  bit_set(TIMSK2, OCIE2A);
  //bit_set(TIMSK2, TOIE2);
  
  task.setup();

  taskSetup(BUS_ADDRESS);
  taskRestart();
}

void loop() {
  if (taskIsEnabled()) {
    task.loop();    
  }

  taskLoop();
}


ISR(TIMER2_COMPA_vect) {
  //gMillis += (1000 / TICK_FREQ);
  task.tick();
  
  /*
  if (gMillis >= 8) gMillis -= 8;
  else {
    bit_set(gFlags, FLAG_TIMEOUT);
  }
  
  if (pinRead(PIN_SWITCH) == LOW) {
    bit_set(gFlags, FLAG_BUTTON);
  }
  */
}
