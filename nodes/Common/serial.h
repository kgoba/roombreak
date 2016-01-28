#pragma once
#include <stdint.h>

#include "fifo.h"

class Serial {
public:
    static void setup(uint32_t baudrate, byte pinTXE, byte pinRXD);
    
    static void enable();
    static void disable();

    static void putChar(char c);
    static char getChar();
    
    static void print(const char * str);
    static void println(const char * str);

    static byte pendingIn();
    static byte pendingOut();
        
    static FIFO<byte, 32> rxFIFO;
    static FIFO<byte, 32> txFIFO;
    
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
