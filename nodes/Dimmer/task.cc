#include <Common/config.h>
#include <Common/serial.h>
#include <Common/modbus.h>
#include <Common/util.h>
#include <Common/audioplayer.h>

#include <util/delay.h>
#include <avr/interrupt.h>

using namespace DimmerConfig;


// internal tick frequency in Hz
#define TICK_FREQ       1000UL
#define PWM_FREQ        100UL
#define PWM2_FREQ       100UL

// IO pins
#define PIN_ZCROSS      12
#define PIN_DIM1        9
#define PIN_DIM2        10

#define PIN_DIM3        5

#define READ_ZCROSS     bit_check(PORTB, PIN_ZCROSS-8)
#define SET_DIM1        bit_set(PORTB, PIN_DIM1-8)
#define SET_DIM2        bit_set(PORTB, PIN_DIM2-8)
#define CLR_DIM1        bit_clear(PORTB, PIN_DIM1-8)
#define CLR_DIM2        bit_clear(PORTB, PIN_DIM2-8)


enum {
  FLAG_DONE,
  FLAG_TIMEOUT
};

volatile byte gFlags;
volatile word gMillis;
volatile word gCounts[3];

byte gDimmerOCR1;
byte gDimmerOCR2;
byte gDimmerTOP;

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
  //gState = kINIT;
  gDimmerTOP = 10;
  gDimmerOCR1 = 5;
  gDimmerOCR2 = 10;

  // setup IO pins
  pinMode(PIN_DIM1, OUTPUT);
  pinMode(PIN_DIM2, OUTPUT);
  pinMode(PIN_DIM3, OUTPUT);
  
  pinWrite(PIN_DIM1, HIGH);
  pinWrite(PIN_DIM2, HIGH);
  
  // Setup Timer0: Fast PWM mode, TOP = OCRA
  TIMER0_SETUP(TIMER0_PWM_PHASE_A, TIMER0_PRESCALER(2*PWM_FREQ));
  TCCR0A |= (1 << COM0B1);
  OCR0A = TIMER0_COUNTS(2*PWM_FREQ) - 1;
  OCR0B = 0;

  // Setup Timer1: Fast PWM mode, TOP = OCRA
  //TIMER1_SETUP(TIMER1_PWM_PF_ICR, TIMER1_PRESCALER(2*PWM2_FREQ));
  //TCCR1A |= (1 << COM1B1) | (1 << COM1A1);
  //ICR1A = TIMER1_COUNTS(2*PWM2_FREQ);
  //OCR0A = 300;
  //OCR0B = 3000;

  // Setup Timer2
  TIMER2_SETUP(TIMER2_FAST_PWM_A, TIMER2_PRESCALER(TICK_FREQ));
  OCR2A = TIMER2_COUNTS(TICK_FREQ) - 1;
  bit_set(TIMSK2, TOIE2);                 // enable timer overflow interrupt

  bit_set(PCICR, PCIE0);    // enable pin change on PORTB
  bit_set(PCMSK0, PIN_ZCROSS - 8);

  serial.setup(BUS_SPEED, PIN_TXE, PIN_RXD);
  serial.enable();  
  bus.setup(BUS_ADDRESS, &busCallback, busParams, BUS_NPARAMS);
}

void loop() {
  // DO SOMETHING
  if (bit_check(gFlags, FLAG_TIMEOUT)) {
    bit_clear(gFlags, FLAG_TIMEOUT);
  }
  
  bus.poll();
  _delay_ms(10);
}

ISR(TIMER2_OVF_vect) {
  static byte pwm = 1;
  gMillis += (1000UL / TICK_FREQ);
  //gMillis += 8;
  if (gMillis >= 100) {
    gMillis -= 100;
    bit_set(gFlags, FLAG_TIMEOUT);    
    OCR0B = pwm;
    pwm++;
    if (pwm > OCR0A || pwm == 0) pwm = 1;
  }
}

ISR(PCINT0_vect) {
  //static byte phase;
  static byte count;
  
  /*
  byte now = READ_ZCROSS;
  
  if (now) {
    if (count == gDimmerTOP) {
      count = 0;
      if (gDimmerOCR1 != 0) SET_DIM1;
      if (gDimmerOCR2 != 0) SET_DIM2;
    }
    else {
      if (count == gDimmerOCR1) CLR_DIM1;
      if (count == gDimmerOCR2) CLR_DIM2;
    }
    //phase = !phase;
    count++;
  }
  */
}