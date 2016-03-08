#include <Common/config.h>
#include <Common/serial.h>
#include <Common/modbus.h>
#include <Common/util.h>

#include <util/delay.h>
#include <avr/interrupt.h>

using namespace ValveConfig;

// valves
// hall - a2, a1, a0
// servo - 9

// internal tick frequency in Hz
#define TICK_FREQ       125UL
#define PPM_FREQ        100UL

#define PIN_PWM         9

// (19, 18), (17, 16), (15, 14)

#define PPM_NEUTRAL_US  1280
#define PPM_MAX_US      (PPM_NEUTRAL_US + 560)
#define PPM_MIN_US      (PPM_NEUTRAL_US - 560)

enum {
  FLAG_DONE,
  FLAG_TIMEOUT
};

volatile byte gFlags;
volatile word gMillis;
volatile word gCounts[3];

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

void servoOn()
{
  pinWrite(PIN_PWM, LOW);
  bit_set(TCCR1A, COM1A1);
}

void servoOff()
{
  bit_clear(TCCR1A, COM1A1);
  pinWrite(PIN_PWM, LOW);
}

void servoSet(word ppm_us)
{
  OCR1A = ((ppm_us * PPM_FREQ) / 1000UL * TIMER1_COUNTS(PPM_FREQ)) / 1000UL;
}

void setup() {
  // setup IO pins
  pinMode(PIN_PWM, OUTPUT);
  
  // Setup Timer0
  // Set Fast PWM mode, TOP = OCRA, prescaler 1024 (64us)
  // PWM period 16ms
  //TIMER0_SETUP(TIMER0_FAST_PWM_A, TIMER0_PRESCALER(PPM_FREQ));
  //TIMER0_SETUP(TIMER0_FAST_PWM_A, 1024UL);
  //TCCR0A = (1 << WGM01) | (1 << WGM00);
  //TCCR0B = (1 << CS02) | (1 << CS00) | (1 << WGM02);
  //OCR0A = 250 - 1;
  //OCR0A = TIMER0_COUNTS(PPM_FREQ) - 1;
  
  int a = 1 / TIMER1_COUNTS(PPM_FREQ);
  
  TIMER1_SETUP(TIMER1_PWM_FAST_ICR, TIMER1_PRESCALER(PPM_FREQ));
  ICR1 = TIMER1_COUNTS(PPM_FREQ) - 1;
  
  // Setup Timer2
  TIMER2_SETUP(TIMER2_FAST_PWM_A, TIMER2_PRESCALER(TICK_FREQ));
  OCR2A = TIMER2_COUNTS(TICK_FREQ) - 1;
  bit_set(TIMSK2, TOIE2);                 // enable timer overflow interrupt

  bit_set(PCICR, PCIE1);    // enable pin change on PORTC
  PCMSK1 = 0x3F;            // pins PC0-PC5

  //gState = kINIT;
  servoOn();
  servoSet(PPM_NEUTRAL_US);

  serial.setup(BUS_SPEED, PIN_TXE, PIN_RXD);
  serial.enable();  
  bus.setup(BUS_ADDRESS, &busCallback, busParams, BUS_NPARAMS);
}

void loop() {
  // DO SOMETHING
  if (bit_check(gFlags, FLAG_TIMEOUT)) {
    bit_clear(gFlags, FLAG_TIMEOUT);
  }

  static word ppm_us = PPM_NEUTRAL_US;
  static word lastCount;
  
  word now = gCounts[0] + gCounts[1] + gCounts[2];
  word diff = now - lastCount;
  lastCount = now;

  ppm_us += 2 * diff;
  if (ppm_us > PPM_MAX_US) ppm_us = PPM_MAX_US;
  else if (ppm_us < PPM_MIN_US) ppm_us = PPM_MIN_US;
  servoSet(ppm_us);
  
  bus.poll();
  _delay_ms(10);
}

ISR(TIMER2_OVF_vect) {
  //static word ppm_us = PPM_NEUTRAL_US;
  //static int16_t ppm_dir = 64;
  static byte ppm_dir = 1;
  
  gMillis += (1000UL / TICK_FREQ);
  //gMillis += 8;
  if (gMillis >= 3000) {
    gMillis -= 3000;
    bit_set(gFlags, FLAG_TIMEOUT);
    
    /*
    if (ppm_dir) {
      servoSet(PPM_MAX_US);      
    }
    else {
      servoSet(PPM_MIN_US);
    }
    ppm_dir = !ppm_dir;
    */
    
    /*
    servoSet(ppm_us);    
    ppm_us += ppm_dir;
    if (ppm_us > PPM_MAX_US) {
      ppm_dir = -ppm_dir;
      ppm_us = PPM_MAX_US;
    }
    else if (ppm_us < PPM_MIN_US) {
      ppm_dir = -ppm_dir;
      ppm_us = PPM_MIN_US;
    }
    */
  }
  
  /*
  if (pinRead(PIN_SWITCH) == LOW) {
    bit_set(gFlags, FLAG_BUTTON);
  }
  */
}

ISR(PCINT1_vect) {
  static byte lastState;
  byte state = (PINC & 0x3F);

  byte tNow = state;
  byte tLast = lastState;
  for (byte idx = 0; idx < 3; idx++) {
    byte qNow = tNow & 0x03;
    byte qLast = tLast & 0x03;
    tNow >>= 2;
    tLast >>= 2;
    if (qNow == qLast) continue;      // no change
    
    // 00 - 01 - 11 - 10
    byte phase = qNow ^ qLast;
    int8_t dir = 0;
    switch (phase) {
      case 1: dir = (qNow == 1 || qNow == 2) ? 1 : -1; break;
      case 2: dir = (qNow == 0 || qNow == 3) ? 1 : -1; break;
      case 3: break;
      case 0: break;
    }
    gCounts[idx] += dir;
  }
  lastState = state;
}