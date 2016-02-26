#pragma once
#include <stdint.h>

#include "util.h"

/*

class ModBus {
public:
  typedef byte (* callback_t) (word number, word *value, bool write);

  static void setup(byte address, callback_t callback, word nBits, word nInBits, word nRegs, word nInRegs);  
  static void poll();

private:
  static byte   _address;
  static word   _interframe_loops;
  static word   _timeout;

  static callback_t   _callback;

  static word   _nBits;
  static word   _nInBits;
  static word   _nRegs;
  static word   _nInRegs;
  
  static byte   _CRCLo;
  static byte   _CRCHi;
  
  static void initCRC();
  static bool checkCRC();
  static void sendCRC();
  
  static byte readByte(bool doCRC = true);
  static word readWord();
  static void sendByte(byte b, bool doCRC = true);
  static void sendWord(word w);  

  static void readBits(byte code, word address, word count);
  static void writeBit(byte code, word address, word value);
  static void writeBits(byte code, word address, word count, byte nBytes);

  static void readRegs(byte code, word address, word count);
  static void writeReg(byte code, word address, word value);
  static void writeRegs(byte code, word address, word count, byte nBytes);

  static void readRegs(word number, word count);
};

*/

#define BUS_DEFAULT_TIMEOUT   20

class NewBus {
public:
  typedef byte (* callback_t) (byte cmd, byte nParams, byte *nResults);

  static void setup(byte address, callback_t callback, byte *argv, byte argc, word timeout = BUS_DEFAULT_TIMEOUT);
  static void poll();
  
private:
  static byte _address;
  static callback_t _callback;
  static word   _timeout;
  static word   _timer;
  static byte * _argv;
  static byte   _argc;
  static byte   _crc;
  static byte   _crcTable[256];
  
  static byte readByte(bool doCRC = true);
  static void sendByte(byte b);
};
