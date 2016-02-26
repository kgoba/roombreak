#pragma once
#include <stdint.h>

#include "fifo.h"

class Serial {
public:
    enum Format { DEC = 0, HEX = 1};
    static void setup(uint32_t baudrate, byte pinTXE, byte pinRXD);
    
    static void enable();
    static void disable();

    static void putChar(char c);
    static char getChar();
    
    static void print(byte b, Format format = Serial::DEC);

    static void print(const char * str);
    static void println(const char * str);
    static void println();

    static byte pendingIn();
    static byte pendingOut();
    
    static uint32_t getBaudrate();
    
    //static FIFO<byte, 32> rxFIFO;
    //static FIFO<byte, 32> txFIFO;
    
    static byte _pinTXE;
    static byte _pinRXD;
    
    static uint32_t _baudrate;
};

/*
void uartBegin(int baudrate, rxFIFO, txFIFO);
void uartPutChar(char c);
char uartGetChar();
void uartFlush();
char uartRxPending();
char uartTxPending();
*/
