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

void digitalWrite(byte pin, PinState state)
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

PinState digitalRead(byte pin)
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

void analogReference()
{
  ADMUX = (ADMUX & 0b00001111) | (1 << REFS0);
}

word analogRead(byte pin)
{
  ADMUX = (ADMUX & 0b11110000) | (pin & 0b00001111);
  ADCSRA |= (1 << ADSC);
  
  while (ADCSRA & (1 << ADSC)) {
    
  }
  return ADC;
}

Serial::Serial() {
    
}

void Serial::begin(int baudrate) {
    int val_UBRR0 = (((F_CPU) + 8UL * (baudrate)) / (16UL * (baudrate)) -1UL);
    UBRR0 = val_UBRR0;
    
    bool use2x = false;
    if (use2x) {
        bit_set(UCSR0A, U2X0);
    }
    else {
        bit_clear(UCSR0A, U2X0);
    }
    
    UCSR0C = bit_mask2(UCSZ01, UCSZ00); /* 8-bit data */
    UCSR0B = bit_mask3(RXEN0, TXEN0, RXCIE0);   /* Enable RX and TX */
}

void Serial::putChar(char c) {
    loop_until_bit_is_set(UCSR0A, UDRE0); /* Wait until data register empty. */
    UDR0 = c;
}

char Serial::getChar() {
    loop_until_bit_is_set(UCSR0A, RXC0); /* Wait until data exists. */
    return UDR0;
}

ISR(USART_RX_vect) {
    byte b = UDR0;
    //
}
