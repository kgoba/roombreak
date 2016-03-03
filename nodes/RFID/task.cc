#include <Common/config.h>
#include <Common/serial.h>
#include <Common/modbus.h>
#include <Common/util.h>

#include <util/delay.h>
#include <avr/interrupt.h>

using namespace RFIDConfig;

#define PIN_SENSE   19, 17, 15, 13, 11, 8
#define PIN_ENABLE  18, 16, 14, 12,  9, 7
#define PIN_BUZZER  5

// valves
// hall - a2, a1, a0
// servo - 9

// internal timing frequency in Hz
#define TICK_FREQ       125

// sensor pin numbers
const byte kPinSense[] = { PIN_SENSE };
const byte kPinEnable[] = { PIN_ENABLE };

enum {
  FLAG_DONE
};

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

void setup() {
  // Setup ENABLE pins
  for (byte iPin = 0; iPin < ARRAY_SIZE(kPinEnable); iPin++) {
    pinMode(kPinEnable[iPin], OUTPUT);
  }

  // Setup Timer2
  // Set CTC mode, TOP = OCRA, prescaler 1024
  // Overflow 125Hz (8ms), overflow interrupt enabled
  TIMER2_SETUP(TIMER2_CTC, TIMER2_PRESCALER(TICK_FREQ));
  OCR2A = TIMER2_COUNTS(TICK_FREQ) - 1;
  TIMSK2 = (1 << TOIE2); 

  //gState = kINIT;

  serial.setup(BUS_SPEED, PIN_TXE, PIN_RXD);
  serial.enable();  
  bus.setup(BUS_ADDRESS, &busCallback, busParams, BUS_NPARAMS);
}


void loop() {  
  // DO SOMETHING

  bus.poll();
  _delay_ms(100);
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
