#include <Common/config.h>
#include <Common/serial.h>
#include <Common/modbus.h>
#include <Common/util.h>
#include <Common/audioplayer.h>
#include <Common/pins.h>

#include <util/delay.h>
#include <avr/interrupt.h>

using namespace DimmerConfig;

#define elif      else if

// internal tick frequency in Hz
#define TICK_FREQ       125UL

#define PWM_FREQ        100UL
#define PWM2_FREQ       100UL

#define PIN_ZCROSS      12

InputPin<12>    pinZeroCross;
OutputPin<9>    pinDimmer1;     // OC1A
OutputPin<10>   pinDimmer2;     // OC1B
PWMPin<5>       pinDimmer3;     // OC0B
PWMPin<6>       pinDimmer4;     // OC0A

enum {
  FLAG_DONE,
  FLAG_TIMEOUT
};

volatile byte gFlags;
volatile word gMillis;
volatile word gCounts[3];


byte gDimmer1Percent;
byte gDimmer2Percent;
byte gDimmer3Percent;
byte gDimmer4Percent;

Serial serial;
NewBus bus;
byte busParams[BUS_NPARAMS];

void setDimmer1(byte percent) {
  if (percent > 100) percent = 100;
  OCR1A = 2500/100 * (100 - percent);
}

void setDimmer2(byte percent) {
  if (percent > 100) percent = 100;
  OCR1B = 2500/100 * (100 - percent);
}

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
    
    case CMD_DIMMER1:
    {
      if (nParams > 0) {
        gDimmer1Percent = busParams[0];
        setDimmer1(gDimmer1Percent);
      }
      *nResults = 1;
      busParams[0] = gDimmer1Percent;
      break;
    }

    case CMD_DIMMER2:
    {
      if (nParams > 0) {
        gDimmer2Percent = busParams[0];
        setDimmer2(gDimmer2Percent);
      }
      *nResults = 1;
      busParams[0] = gDimmer2Percent;
      break;
    }
    
    case CMD_DIMMER3:
    {
      if (nParams > 0) {
        gDimmer3Percent = busParams[0];
        //pinDimmer3.setPWMPercent(PWM_FREQ, gDimmer3Percent);
      }
      *nResults = 1;
      busParams[0] = gDimmer3Percent;
      break;      
    }
    
    case CMD_DIMMER4:
    {
      if (nParams > 0) {
        gDimmer4Percent = busParams[0];
        //pinDimmer4.setPWMPercent(PWM_FREQ, gDimmer4Percent);
      }
      *nResults = 1;
      busParams[0] = gDimmer4Percent;
      break;      
    }

    default:
    break;
  }
  return 0;
}

void setup() {
  //gState = kINIT;

  pinZeroCross.setup();
  pinDimmer1.setup();
  pinDimmer2.setup();
  pinDimmer3.setup();
  pinDimmer4.setup();
  
  //pinDimmer1.on();
  //pinDimmer2.on();
  
  pinDimmer3.enable();
  pinDimmer4.enable();
      
  // Setup Timer0: Fast PWM mode, TOP = OCRA
  TIMER0_SETUP(TIMER0_PWM_PHASE, 8);
  TCCR0A |= (1 << COM0B1);
  OCR0A = 0;
  //TIMER0_COUNTS(2*PWM_FREQ) - 1;
  OCR0B = 0;

  // Setup Timer1: TOP = ICR
  //ICR1 = TIMER1_COUNTS(PWM2_FREQ);
  //OCR1A = ICR1 / 2;
  //OCR1B = ICR1 / 2;
  //TIMER1_SETUP(TIMER1_PWM_FAST_ICR, TIMER1_PRESCALER(PWM2_FREQ));
  TIMER1_SETUP(TIMER1_PWM_FAST_ICR, 64UL);
  TCCR1A |= (1 << COM1B1) | (1 << COM1B0);
  TCCR1A |= (1 << COM1A1) | (1 << COM1A0);
  ICR1 = 2500;
  
  setDimmer1(70);
  setDimmer2(30);
  //OCR1A = ICR1/2;
  //OCR1B = 0;
  //bit_set(TIMSK1, OCIE1A);
  //bit_set(TIMSK1, OCIE1B);
  //bit_set(TIMSK1, TOIE1);

  // Setup Timer2
  TIMER2_SETUP(TIMER2_FAST_PWM_A, TIMER2_PRESCALER(TICK_FREQ));
  OCR2A = TIMER2_COUNTS(TICK_FREQ) - 1;
  bit_set(TIMSK2, TOIE2);                 // enable timer overflow interrupt

  pinZeroCross.enablePCInt();
  //bit_set(PCICR, PCIE0);    // enable pin change on PORTB
  //bit_set(PCMSK0, PIN_ZCROSS - 8);

  serial.setup(BUS_SPEED, PIN_TXE, PIN_RXD);
  serial.enable();  
  bus.setup(BUS_ADDRESS, &busCallback, busParams, BUS_NPARAMS);
}

void loop() {
  static byte dir = 1;
  static byte phase;
  
  // DO SOMETHING
  if (bit_check(gFlags, FLAG_TIMEOUT)) {
    bit_clear(gFlags, FLAG_TIMEOUT);
  }
  
  //pinDimmer3.setPWMPercent(PWM_FREQ, gDimmer3Percent);
  //pinDimmer4.setPWMPercent(PWM_FREQ, gDimmer4Percent);
  OCR0A = gDimmer3Percent * 255 / 100;
  OCR0B = gDimmer4Percent * 255 / 100;

  /*
  gDimmer3Percent += dir;
  if (gDimmer3Percent >= 100) {
    dir = -dir;
  }
  elif (gDimmer3Percent == 0) {
    dir = -dir;
  }
  */
  
  bus.poll();
  _delay_ms(50);
}

ISR(TIMER2_OVF_vect) {
  gMillis += (1000UL / TICK_FREQ);

  if (gMillis >= 1000) {
    gMillis -= 1000;
    bit_set(gFlags, FLAG_TIMEOUT);    
  }
  
  
  /*
  static word dir = -4;
  
  word min = ICR1 / 6;
  word max = ICR1 / 6 * 5;
  OCR1A += dir;
  OCR1B += dir;
  
  if (OCR1A > max) {
    dir = -dir;
    OCR1A = max;
    OCR1B = max;
  }
  if (OCR1A < min) {
    dir = -dir;
    OCR1A = min;
    OCR1B = min;
  }
  */
}

ISR(PCINT0_vect) {  
  //if (pinZeroCross.get()) {
      
  //if (TCNT1 > 20) ICR1 -= 5;
  //if (ICR1 - TCNT1 > 20) ICR1 += 5;

  if (pinZeroCross.get()) {
    TCNT1 = ICR1 - 2;
  }
}
