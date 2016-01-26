#pragma once
#include <stdint.h>

#include "fifo.h"

class Serial {
public:
    static void setup(int baudrate, byte pinTXE, byte pinRXD);
    
    static void enable();
    static void disable();

    static void putChar(char c);
    static char getChar();

    static byte pendingIn();
    static byte pendingOut();
        
    static FIFO<byte, 8> rxFIFO;
    static FIFO<byte, 8> txFIFO;
    
    static byte _pinTXE;
    static byte _pinRXD;
};

/*
void uartBegin(int baudrate, rxFIFO, txFIFO);
void uartPutChar(char c);
char uartGetChar();
void uartFlush();
char uartRxPending();
char uartTxPending();
*/
