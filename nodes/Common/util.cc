#include <avr/interrupt.h>

#include "util.h"


void pinMode(byte pin, PinMode mode)
{
  byte bit;
  volatile uint8_t * pPort;
  if (pin < 8) {
    bit = pin;
    pPort = &DDRD;
  }
  else if (pin < 14) {
    bit = pin - 8;
    pPort = &DDRB;
  }
  else {
    bit = pin - 14;
    pPort = &DDRC;
  }
  byte mask = (1 << bit);
  switch (mode) {
    case INPUT:
      *pPort &= (~mask);
      break;
    case OUTPUT:
      *pPort |= mask;
      break;
  }
}

void pinWrite(byte pin, PinState state)
{
  byte bit;
  volatile uint8_t * pPort;
  if (pin < 8) {
    bit = pin;
    pPort = &PORTD;
  }
  else if (pin < 14) {
    bit = pin - 8;
    pPort = &PORTB;
  }
  else {
    bit = pin - 14;
    pPort = &PORTC;
  }
  byte mask = (1 << bit);
  switch (state) {
    case LOW:
      *pPort &= (~mask);
      break;
    case HIGH:
      *pPort |= mask;
      break;
  }
}

PinState pinRead(byte pin)
{
  byte bit;
  volatile uint8_t * pPort;
  if (pin < 8) {
    bit = pin;
    pPort = &PIND;
  }
  else if (pin < 14) {
    bit = pin - 8;
    pPort = &PINB;
  }
  else {
    bit = pin - 14;
    pPort = &PINC;
  }
  byte mask = (1 << bit);
  return ((*pPort) & mask) ? HIGH : LOW;
}

void adcSetup()
{
  
}

void adcEnable()
{
  bit_set(ADCSRA, ADEN);
}

void adcReference()
{
  ADMUX = (ADMUX & 0b00001111) | bit_mask1(REFS0);
}

word adcRead(byte pin)
{
  ADMUX = (ADMUX & 0b11110000) | (pin & 0b00001111);
  bit_set(ADCSRA, ADSC);
  
  while (bit_check(ADCSRA, ADSC)) {}
  return ADC;
}


static byte a;
static byte b;
static byte c;
static byte x;

void rng_init(byte s1, byte s2, byte s3) //Can also be used to seed the rng with more entropy during use.
{
  //XOR new entropy into key state
  a ^=s1;
  b ^=s2;
  c ^=s3;
}

byte rng_get()
{
  x++;               //x is incremented every round and is not affected by any other variable
  a = (a^c^x);       //note the mix of addition and XOR
  b = (b+a);         //And the use of very few instructions
  c = (c+(b>>1)^a);  //the right shift is to ensure that high-order bits from b can affect  
  return c;          //low order bits of other variables
}