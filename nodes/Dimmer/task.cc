#include <Common/config.h>
#include <Common/task.h>
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
  FLAG_TIMEOUT,
  FLAG_KABOOM
};

volatile byte gFlags;
volatile word gMillis;
volatile word gSecondsKaboom;
volatile word gCounts[3];


volatile byte gDimmer1Ramp;
volatile byte gDimmer2Ramp;
volatile byte gDimmer3Ramp;
volatile byte gDimmer4Ramp;

byte gDimmer1Percent;
byte gDimmer2Percent;
byte gDimmer3Percent;
byte gDimmer4Percent;

void setDimmer1(byte percent) {
  if (percent > 100) percent = 100;
  gDimmer1Percent = percent;
}

void setDimmer2(byte percent) {
  if (percent > 100) percent = 100;
  gDimmer2Percent = percent;
}

void setDimmer3(byte percent) {
  if (percent > 100) percent = 100;
  gDimmer3Percent = percent;
}

void setDimmer4(byte percent) {
  if (percent > 100) percent = 100;
  gDimmer4Percent = percent;
}

void taskRestart() {
  gDimmer1Percent = 100;
  gDimmer2Percent = 100;
  gDimmer3Percent = 100;
  gDimmer4Percent = 100;
}

void taskComplete() {
  
}

byte taskIsDone() {
  return 0;
}

byte taskCallback(byte cmd, byte nParams, byte *nResults, byte *busParams)
{
  switch (cmd) {
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
    
    case CMD_KABOOM:
    {
      if (nParams > 0) {
        gSecondsKaboom = busParams[0];
        *nResults = nParams;
      }
      else gSecondsKaboom = 10;
      rng_init(gMillis >> 8, gMillis & 0xFF, 73);
      bit_set(gFlags, FLAG_KABOOM);        
      break;
    }
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
  
  // Setup Timer2
  TIMER2_SETUP(TIMER2_FAST_PWM_A, TIMER2_PRESCALER(TICK_FREQ));
  OCR2A = TIMER2_COUNTS(TICK_FREQ) - 1;
  bit_set(TIMSK2, TOIE2);                 // enable timer overflow interrupt

  pinZeroCross.enablePCInt();
  //bit_set(PCICR, PCIE0);    // enable pin change on PORTB
  //bit_set(PCMSK0, PIN_ZCROSS - 8);

  taskRestart();
  taskSetup(BUS_ADDRESS);
}

void loop() 
{
  if (bit_check(gFlags, FLAG_KABOOM)) {
  }
  else {
    // Normal operation
    
    OCR1A = 2500/100 * (100 - gDimmer1Ramp);
    OCR1B = 2500/100 * (100 - gDimmer2Ramp);

    OCR0A = gDimmer3Ramp * 255 / 100;
    OCR0B = gDimmer4Ramp * 255 / 100;    
  }
    
  taskLoop();
}

ISR(TIMER2_OVF_vect) {  
  gMillis += (1000UL / TICK_FREQ);

  if (gMillis >= 1000) {
    gMillis -= 1000;
    bit_set(gFlags, FLAG_TIMEOUT);    
    
    
    if (bit_check(gFlags, FLAG_KABOOM)) {
      if (gSecondsKaboom > 0) {
        gSecondsKaboom--;
      }
      else {
        bit_clear(gFlags, FLAG_KABOOM);
        gDimmer1Ramp = 0;
        gDimmer2Ramp = 0;
        gDimmer1Percent = 100;
        gDimmer2Percent = 100;
      }
    }
  }
  
  static word millis2;
  millis2 += (1000UL / TICK_FREQ);
  if (millis2 >= 64) {
    millis2 -= 64;

    if (gDimmer1Ramp > gDimmer1Percent) gDimmer1Ramp--;
    else if (gDimmer1Ramp < gDimmer1Percent) gDimmer1Ramp++;
    if (gDimmer2Ramp > gDimmer2Percent) gDimmer2Ramp--;
    else if (gDimmer2Ramp < gDimmer2Percent) gDimmer2Ramp++;
    if (gDimmer3Ramp > gDimmer3Percent) gDimmer3Ramp--;
    else if (gDimmer3Ramp < gDimmer3Percent) gDimmer3Ramp++;
    if (gDimmer4Ramp > gDimmer4Percent) gDimmer4Ramp--;
    else if (gDimmer4Ramp < gDimmer4Percent) gDimmer4Ramp++;    
    
    if (bit_check(gFlags, FLAG_KABOOM)) {
      // special FX    
      if (rng_get() < 40) OCR1A = 100;
      else OCR1A = 2400;
    
      if (rng_get() < 40) OCR1B = 100;
      else OCR1B = 2400;

      if (rng_get() < 40) OCR0A = 255;
      else OCR0A = 0;

      if (rng_get() < 40) OCR0B = 255;
      else OCR0B = 0;
    }
  }
}

ISR(PCINT0_vect) {  
  //if (pinZeroCross.get()) {
      
  //if (TCNT1 > 20) ICR1 -= 5;
  //if (ICR1 - TCNT1 > 20) ICR1 += 5;

  if (pinZeroCross.get()) {
    TCNT1 = ICR1 - 2;
  }
}
