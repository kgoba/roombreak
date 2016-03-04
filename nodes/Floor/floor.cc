#include <Common/config.h>
#include <Common/serial.h>
#include <Common/modbus.h>
#include <Common/util.h>

#include <util/delay.h>
#include <avr/interrupt.h>

using namespace FloorConfig;

#define PIN_SENSE       19, 18, 17, 16, 15, 14, 7, 8, 10, 11

// rfid
// data 19, en 18, 
// data 17, en 16
// data 15, en 14
// data 13, en 12
// data 11, en  9
// data  8, en  7
// buzzer 5

// valves
// hall - a2, a1, a0
// servo - 9

// internal timing frequency in Hz
#define TICK_FREQ       125

// sensor pin numbers
const byte kPinSense[] = { PIN_SENSE };

// audio player track selection pin numbers starting from LSB
//const byte PIN_PLAYER[] = {};

enum {
  FLAG_DONE
};

enum {
  kINIT,
  kCLOSED,
  kOPENING,
  kCLOSING
};

volatile byte gFlags;
volatile word gMillis;
byte gState;

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

/*
void audioPlay(byte track) {
  byte mask = 1;
  for (byte idx = 0; idx < ARRAY_SIZE(PIN_PLAYER); idx++) {
    pinWrite(PIN_PLAYER[idx], (track & mask) ? HIGH : LOW);
    mask <<= 1;
  }
}
*/

void setup() {
  // Setup Timer2
  // Set CTC mode, TOP = OCRA, prescaler 1024
  // Overflow 125Hz (8ms), overflow interrupt enabled
  TCCR2A = (1 << WGM21) | (1 << WGM20);
  TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20) | (1 << WGM22);
  TIMSK2 = (1 << TOIE2); 
  OCR2A = (byte)(F_CPU / (1024UL * TICK_FREQ)) - 1;

  gState = kINIT;

  serial.setup(BUS_SPEED, PIN_TXE, PIN_RXD);
  serial.enable();  
  bus.setup(BUS_ADDRESS, &busCallback, busParams, BUS_NPARAMS);
}


void loop() {  
  // DO SOMETHING

  bus.poll();
  _delay_ms(50);
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