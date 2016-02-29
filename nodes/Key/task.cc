#include <Common/config.h>
#include <Common/serial.h>
#include <Common/modbus.h>
#include <Common/util.h>

#include <util/delay.h>
#include <avr/interrupt.h>

// internal timing frequency in Hz
#define TICK_FREQ       125

// IO pin definitions
#define PIN_PWM         5
#define PIN_SWITCH      6

using namespace KeyConfig;

enum {
  FLAG_BUTTON,
  FLAG_TIMEOUT
};

enum {
  kINIT,
  kCLOSED,
  kOPENING,
  kCLOSING
};

volatile byte gFlags;
volatile word gMillis;
bool gTaskDone;
byte gState;


Serial serial;
NewBus bus;
byte busParams[BUS_NPARAMS];

byte busCallback(byte cmd, byte nParams, byte *nResults)
{
  switch (cmd) {
    default:
    break;
  }
  return 0;
}


void servoOn()
{
  bit_set(TCCR0A, COM0B1);
}

void servoOff()
{
  bit_clear(TCCR0A, COM0B1);
}

void servoSet(word ppm_us)
{
  OCR0B = ppm_us / 64;
}

void setup() {
  // setup IO pins
  pinMode(PIN_PWM, OUTPUT);
  pinWrite(PIN_SWITCH, HIGH);

  // Setup Timer0
  // Set Fast PWM mode, TOP = OCRA, prescaler 1024 (64us)
  // PWM period 16ms
  TCCR0A = (1 << WGM01) | (1 << WGM00);
  TCCR0B = (1 << CS02) | (1 << CS00) | (1 << WGM02);
  OCR0A = 250 - 1;
  
  // Setup Timer2
  // Set CTC mode, TOP = OCRA, prescaler 1024
  // Overflow 125Hz (8ms), overflow interrupt enabled
  TCCR2A = (1 << WGM21) | (1 << WGM20);
  TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20) | (1 << WGM22);
  TIMSK2 = (1 << TOIE2); 
  OCR2A = (byte)(F_CPU / (1024UL * TICK_FREQ)) - 1;

  gMillis = HOLD_MS;
  servoOn();
  servoSet(PPM_CLOSED_US);
  gState = kINIT;

  serial.setup(BUS_SPEED, PIN_TXE, PIN_RXD);
  serial.enable();  
  bus.setup(BUS_ADDRESS, &busCallback, busParams, BUS_NPARAMS);
}

void loop() {  
  if (!gTaskDone) {
    if (gState == kINIT && bit_check(gFlags, FLAG_TIMEOUT)) {
      servoOff();
      gState = kCLOSED;
    }
    else if (gState == kCLOSED && bit_check(gFlags, FLAG_BUTTON)) {
      servoOn();
      servoSet(PPM_OPEN_US);
      gMillis = HOLD_MS;
      bit_clear(gFlags, FLAG_TIMEOUT);
      gState = kOPENING;
    }
    else if (gState == kOPENING && bit_check(gFlags, FLAG_TIMEOUT)) {
      servoSet(PPM_CLOSED_US);
      gMillis = HOLD_MS;
      bit_clear(gFlags, FLAG_TIMEOUT);
      gState = kCLOSING;
    }
    else if (gState == kCLOSING && bit_check(gFlags, FLAG_TIMEOUT)) {
      bit_clear(gFlags, FLAG_BUTTON);
      servoOff();
      gState = kCLOSED;
      gTaskDone = true;
    } 
  }

  bus.poll();
  _delay_ms(100);
}

ISR(TIMER2_OVF_vect) {
  if (gMillis >= 8) gMillis -= 8;
  else {
    bit_set(gFlags, FLAG_TIMEOUT);
  }
  
  if (pinRead(PIN_SWITCH) == LOW) {
    bit_set(gFlags, FLAG_BUTTON);
  }
}