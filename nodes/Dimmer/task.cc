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
#define PWM2_FREQ       40UL

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

byte dimmer3Percent;
byte dimmer4Percent;

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
    
    case CMD_DIMMER1:
    {
      if (nParams > 0) {
        pinDimmer1.set(nResults[0]);
      }
      nResults[0] = pinDimmer1.get() ? 0x01 : 0x00;
      break;
    }

    case CMD_DIMMER2:
    {
      if (nParams > 0) {
        pinDimmer2.set(nResults[0]);
      }
      nResults[0] = pinDimmer2.get() ? 0x01 : 0x00;
      break;
    }
    
    case CMD_DIMMER3:
    {
      if (nParams > 0) {
        dimmer3Percent = nResults[0];
        pinDimmer3.setPWMPercent(PWM_FREQ, dimmer3Percent);
      }
      nResults[0] = dimmer3Percent;
      break;      
    }
    
    case CMD_DIMMER4:
    {
      if (nParams > 0) {
        dimmer4Percent = nResults[0];
        pinDimmer4.setPWMPercent(PWM_FREQ, dimmer4Percent);
      }
      nResults[0] = dimmer4Percent;
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
  
  pinDimmer1.on();
  pinDimmer2.on();
  
  pinDimmer3.enable();
  pinDimmer4.enable();
    
  //pinWrite(PIN_DIM1, HIGH);
  //pinWrite(PIN_DIM2, HIGH);
  
  // Setup Timer0: Fast PWM mode, TOP = OCRA
  //TIMER0_SETUP(TIMER0_PWM_PHASE_A, TIMER0_PRESCALER(2*PWM_FREQ));
  //TCCR0A |= (1 << COM0B1);
  //OCR0A = TIMER0_COUNTS(2*PWM_FREQ) - 1;
  //OCR0B = 0;

  // Setup Timer1: TOP = ICR
  ICR1 = TIMER1_COUNTS(PWM2_FREQ);
  OCR1A = ICR1 / 2;
  OCR1B = ICR1 / 2;
  TIMER1_SETUP(TIMER1_PWM_FAST_ICR, TIMER1_PRESCALER(PWM2_FREQ));
  //TCCR1A |= (1 << COM1B1) | (1 << COM1B0);
  //TCCR1A |= (1 << COM1A1) | (1 << COM1A0);
  bit_set(TIMSK1, OCIE1B);
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
  
  // DO SOMETHING
  if (bit_check(gFlags, FLAG_TIMEOUT)) {
    bit_clear(gFlags, FLAG_TIMEOUT);
  }
  
  pinDimmer3.setPWMPercent(PWM_FREQ, dimmer3Percent);

  dimmer3Percent += dir;
  if (dimmer3Percent >= 100) {
    dir = -dir;
  }
  elif (dimmer3Percent == 0) {
    dir = -dir;
  }
  
  bus.poll();
  _delay_ms(50);
}

ISR(TIMER2_OVF_vect) {
  gMillis += (1000UL / TICK_FREQ);

  if (gMillis >= 1000) {
    gMillis -= 1000;
    bit_set(gFlags, FLAG_TIMEOUT);    
  }
}

ISR(PCINT0_vect) {
  static byte cnt;
  
  if (pinZeroCross.get()) {
    cnt++;
    if (cnt == 4) {
      cnt = 0;
      
      //pinDimmer2.off();
      TCNT1 = ICR1 - 10;
    }
  }
}

ISR(TIMER1_COMPB_vect) {
  //pinDimmer2.on();
}

ISR(TIMER1_OVF_vect) {
  //pinDimmer2.off();
}
