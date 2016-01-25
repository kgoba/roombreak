#include <avr/interrupt.h>
#include <avr/io.h>

#include "serial.h"

FIFO<byte, 8> Serial::rxFIFO;
FIFO<byte, 8> Serial::txFIFO;
byte Serial::_pinTXE;
byte Serial::_pinRXD;

void Serial::begin(int baudrate, byte pinTXE, byte pinRXD) {
    _pinTXE = pinTXE;
    _pinRXD = pinRXD;

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
    //loop_until_bit_is_set(UCSR0A, UDRE0); /* Wait until data register empty. */
    //UDR0 = c;
    if (txFIFO.push(c)) {
        if (!bit_check(UCSR0B, UDRIE0)) {
            digitalWrite(_pinTXE, HIGH);
            digitalWrite(_pinRXD, HIGH);
            bit_set(UCSR0B, UDRIE0);
        }
    }
}

char Serial::getChar() {
    //loop_until_bit_is_set(UCSR0A, RXC0); /* Wait until data exists. */
    //return UDR0;

    //while (rxFIFO.empty()) {}    
    return rxFIFO.pop();
}

byte Serial::pendingIn() {
    return rxFIFO.available();
}

byte Serial::pendingOut() {
    return txFIFO.available();
}

ISR(USART_RX_vect) {
    byte b = UDR0;
    Serial::rxFIFO.push(b);
}

ISR(USART_UDRE_vect) {
    byte b = Serial::txFIFO.pop();
    UDR0 = b;
    if (Serial::txFIFO.empty()) {
        bit_clear(UCSR0B, UDRIE0);
    }
}

ISR(USART_TX_vect) {
    digitalWrite(Serial::_pinTXE, LOW);
    digitalWrite(Serial::_pinRXD, LOW);    
}
