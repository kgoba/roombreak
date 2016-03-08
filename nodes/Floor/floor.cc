#include <Common/config.h>
#include <Common/serial.h>
#include <Common/modbus.h>
#include <Common/util.h>
#include <Common/audioplayer.h>

#include <util/delay.h>
#include <avr/interrupt.h>

using namespace FloorConfig;

#define PIN_SENSE       19, 18, 17, 16, 15, 14, 7, 8, 10, 11

// IO expander control pins
#define PIN_SDA           13
#define PIN_CLK           12

// audio player track selection pins (on I/O expander)
#define XPIN_TRSEL0  15
#define XPIN_TRSEL1  14
#define XPIN_TRSEL2  13
#define XPIN_TRSEL3  17
#define XPIN_TRSEL4  16

WS2803S ioExpander(PIN_SDA, PIN_CLK);
AudioPlayer player1(ioExpander, XPIN_TRSEL0, XPIN_TRSEL1, XPIN_TRSEL2, XPIN_TRSEL3, XPIN_TRSEL4);

const byte pinSense[] = { PIN_SENSE };

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
  for (byte idx = 0; idx < ARRAY_SIZE(pinSense); idx++)
    pinWrite(pinSense[idx], HIGH);
  
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
  
  ioExpander.setup();
  player1.setup();
}


void loop() {
  static byte sound = 0;
  
  // DO SOMETHING
  bool found = false;
  for (byte idx = 0; idx < ARRAY_SIZE(pinSense); idx++) {
    if (pinRead(pinSense[idx]) == LOW) {
      found = true;
      break;
    }
  }
  if (found) {
    player1.play(1);
    //_delay_ms(10);
    //player1.stop();
  }
  else {
    player1.stop();    
  }

  bus.poll();
  _delay_ms(10);
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
