#include <Common/config.h>
#include <Common/serial.h>
#include <Common/modbus.h>
#include <Common/util.h>
#include <Common/pins.h>

#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>

#include "task.h"

using namespace MapConfig;

// Timer tick frequency in Hz
#define TICK_FREQ   125

#define PIN_BUZZER  5 // PWM
/*
Uzdevums:

Savadīt 4 biļetes pareizā secībā. 
Biļetes nr. ir 4 simboli, ko ievada ar keypad. Nospiežot pogu, ir skaņas indikācija.
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

PWMPin<5> pinBuzzer;

void setup() {
  pinBuzzer.setup(440);
  pinBuzzer.set(true);
  _delay_ms(500);
  pinBuzzer.set(false);
  
  //pinMode(PIN_BUZZER, OUTPUT);
  // Setup Timer0
  // Set CTC mode, TOP = OCRA, prescaler 256 (4us)
  TCCR0A = (1 << WGM01);
  TCCR0B = (1 << CS02);

  // Setup Timer2
  // Set CTC mode, TOP = OCRA, prescaler 1024
  // Overflow 125Hz (8ms), overflow interrupt enabled
  TCCR2A = (1 << WGM21) | (1 << WGM20);
  TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20) | (1 << WGM22);
  TIMSK2 = (1 << TOIE2); 
  OCR2A = (byte)(F_CPU / (1024UL * TICK_FREQ)) - 1;
  
  task.setup();

  serial.setup(BUS_SPEED, PIN_TXE, PIN_RXD);
  serial.enable();  
  bus.setup(BUS_ADDRESS, &busCallback, busParams, BUS_NPARAMS);
}

void loop() {
  task.loop();
  
  bus.poll();
  _delay_ms(20);
}


ISR(TIMER2_OVF_vect) {
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
