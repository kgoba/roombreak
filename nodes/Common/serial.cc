#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#include "serial.h"

//volatile static FIFO<byte, 32> rxFIFO;
//volatile static FIFO<byte, 32> txFIFO;

volatile FIFO(rxFIFO, byte, 32);
volatile FIFO(txFIFO, byte, 32);

byte Serial::_pinTXE;
byte Serial::_pinRXD;
uint32_t Serial::_baudrate;

void Serial::setup(uint32_t baudrate, byte pinTXE, byte pinRXD) {
    _pinTXE = pinTXE;
    _pinRXD = pinRXD;
    _baudrate = baudrate;

    word val_UBRR0 = (F_CPU + 8UL * baudrate) / (16UL * baudrate) - 1;
    UBRR0 = val_UBRR0;

    bool use2x = false;
    if (use2x) {
        bit_set(UCSR0A, U2X0);
    }
    else {
        bit_clear(UCSR0A, U2X0);
    }
    
    UCSR0C = bit_mask2(UCSZ01, UCSZ00); /* 8-bit data */
    pinMode(pinTXE, OUTPUT);
    pinMode(pinRXD, OUTPUT);
}

void Serial::enable() {
    UCSR0B = bit_mask2(RXEN0, TXEN0) | bit_mask2(RXCIE0, TXCIE0);   /* Enable RX and TX */
}

void Serial::disable() {
    UCSR0B = 0;
}

uint32_t Serial::getBaudrate() {
  return _baudrate;
}

void Serial::print(const char * str) {
  while (*str) {
    putChar(*str++);
  }
}

void Serial::print(byte b, Format format) {
  switch (format) {
    case DEC:
    {
      byte d0 = b % 10;
      byte d1 = (b / 10) % 10;
      byte d2 = (b / 100);
      putChar('0' + d2);
      putChar('0' + d1);
      putChar('0' + d0);          
    }
    case HEX:
    {
      byte d0 = (b & 0x0F);
      byte d1 = (b >> 4);
      putChar(d1 + ((d1 > 9) ? ('A' - 10) : '0'));
      putChar(d0 + ((d0 > 9) ? ('A' - 10) : '0'));
    }
  }
}

void Serial::println(const char * str) {
  print(str);
  println();
}

void Serial::println() {
  putChar('\r');
  putChar('\n');
}

void Serial::putChar(char c) {
  /*
  loop_until_bit_is_set(UCSR0A, UDRE0); // Wait until data register empty. 
  digitalWrite(_pinTXE, HIGH);
  digitalWrite(_pinRXD, HIGH);
  _delay_us(10);
  UDR0 = c;
  */
  
  while (FIFO_FULL(txFIFO)) {}
  cli();
  FIFO_PUSH(txFIFO, c);
  sei();
  if (!bit_check(UCSR0B, UDRIE0)) {
      digitalWrite(_pinTXE, HIGH);
      digitalWrite(_pinRXD, HIGH);
      bit_set(UCSR0B, UDRIE0);
  }
  
  /*
  loop_until_bit_is_set(UCSR0A, UDRE0); // Wait until data register empty. 
  _delay_us(10);
  digitalWrite(Serial::_pinTXE, LOW);
  digitalWrite(Serial::_pinRXD, LOW);    
  */
}

char Serial::getChar() {
  //loop_until_bit_is_set(UCSR0A, RXC0); /* Wait until data exists. */
  //return UDR0;

  //while (rxFIFO.empty()) {}
  cli();
  char c = FIFO_HEAD(rxFIFO);
  FIFO_POP(rxFIFO);
  sei();
  return c;
}

byte Serial::pendingIn() {
  return FIFO_COUNT(rxFIFO);
}

byte Serial::pendingOut() {
  //return txFIFO.available();
  return FIFO_COUNT(txFIFO);
  //return 0;
}

ISR(USART_RX_vect) {
  byte b = UDR0;
  FIFO_PUSH(rxFIFO, b);
}

ISR(USART_UDRE_vect) {
  byte b = FIFO_HEAD(txFIFO);
  FIFO_POP(txFIFO);
  UDR0 = b;
  if (FIFO_EMPTY(txFIFO)) {
    bit_clear(UCSR0B, UDRIE0);
  }
}

ISR(USART_TX_vect) {
  digitalWrite(Serial::_pinTXE, LOW);
  digitalWrite(Serial::_pinRXD, LOW);    
}
