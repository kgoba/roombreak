#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <avr/io.h>
#include <util/delay.h>

#include "config.h"
#include "serial.h"
#include "modbus.h"

/*

#define MIN_FRAME_LENGTH 4

#define FC_READ_INPUTS  2
#define FC_READ_COILS   1
#define FC_WRITE_COIL   5
#define FC_WRITE_COILS  15
#define FC_READ_IREGS   4
#define FC_READ_HREGS   3
#define FC_WRITE_HREG   6
#define FC_WRITE_HREGS  16

#define START_BITS      00001
#define START_INBITS    10001
#define START_REGS      40001
#define START_INREGS    30001

#define TIMEOUT         0xFF00

byte ModBus::_address;

word ModBus::_interframe_loops;

word   ModBus::_nBits;
word   ModBus::_nInBits;
word   ModBus::_nRegs;
word   ModBus::_nInRegs;

byte   ModBus::_CRCLo;
byte   ModBus::_CRCHi;

static const byte auxCRCHi[] = {
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40
};

static const byte auxCRCLo[] = {
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7,
    0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E,
    0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9,
    0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC,
    0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
    0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32,
    0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D,
    0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 
    0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF,
    0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1,
    0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4,
    0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 
    0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA,
    0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
    0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0,
    0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97,
    0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E,
    0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89,
    0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83,
    0x41, 0x81, 0x80, 0x40
};

void ModBus::setup(byte address, callback_t callback, word nBits, word nInBits, word nRegs, word nInRegs) {
  _address = address;
  uint32_t baudrate = Serial::getBaudrate();
  _interframe_loops = 1 + (100000*35UL) / baudrate;
  _timeout = _interframe_loops;
  _nBits = nBits;
  _nInBits = nInBits;
  _nRegs = nRegs;
  _nInRegs = nInRegs;
  _callback = callback;
}

//void ModBus::initCRC() {
//}

//void ModBus::addCRC(byte b) {
//}

void ModBus::sendByte(byte b, bool doCRC) {
  Serial::putChar(b);
  
  if (doCRC) {
    //addCRC(b);
    byte index = _CRCLo ^ b;
    _CRCLo = _CRCHi ^ auxCRCHi[index];
    _CRCHi = auxCRCLo[index];    
  }
}

byte ModBus::readByte(bool doCRC) {
  if (!_timeout) {
    return 0;
  }
  while (!Serial::pendingIn()) {
    _delay_us(10);
    _timeout--;
    if (!_timeout) {
      return 0;
    }
  }
  _timeout = _interframe_loops;
  byte b = Serial::getChar();
  
  if (doCRC) {
    //addCRC(b);
    byte index = _CRCLo ^ b;
    _CRCLo = _CRCHi ^ auxCRCHi[index];
    _CRCHi = auxCRCLo[index];    
  }
  
  return b;
}

word ModBus::readWord() {
  word hi = readByte();
  byte lo = readByte();
  return (hi << 8) | lo;
}

bool ModBus::checkCRC() {
  byte hi = readByte(false);
  byte lo = readByte(false);
  if (!_timeout) return false;
  return (hi == _CRCHi) && (lo == _CRCLo);
}

void ModBus::sendCRC() {
  sendByte(_CRCHi, false);
  sendByte(_CRCLo, false);
}

void ModBus::initCRC() {
  _CRCHi = 0xFF;
  _CRCLo = 0xFF;
}

void ModBus::poll() {
  if (Serial::pendingIn() < MIN_FRAME_LENGTH) return;
  _timeout = _interframe_loops;
  initCRC();
  
  byte address = readByte();
  byte code = readByte();
  switch (code) {
    case FC_READ_INPUTS: 
    case FC_READ_COILS: 
    {
      word address = readWord();
      word count = readWord();
      readBits(code, address, count);
      break;      
    }
    case FC_WRITE_COIL: 
    {
      word address = readWord();
      word value = readWord();
      writeBit(code, address, value);
      break;
    }
    case FC_WRITE_COILS: 
    {
      word address = readWord();
      word count = readWord();
      byte nBytes = readByte();
      writeBits(code, address, count, nBytes);
      break;
    }
    case FC_READ_IREGS:
    case FC_READ_HREGS:
    {
      word address = readWord();
      word count = readWord();
      readRegs(code, address, count);
      break;
    }
    case FC_WRITE_HREG:
    {
      word address = readWord();
      word value = readWord();
      writeReg(code, address, value);
      break;
    }
    case FC_WRITE_HREGS:
    {
      word address = readWord();
      word count = readWord();
      byte nBytes = readByte();
      writeRegs(code, address, count, nBytes);
      break;
    }
    default:
    {
      while (_timeout) {
        byte b = readByte();
      }
      break;
    }
  }
}

void ModBus::readBits(byte code, word address, word count) {
  if (!checkCRC()) return;
  initCRC();
  
  byte nBytes = (count + 7) / 8;
  sendByte(code);
  sendByte(nBytes);
  
  word number = address + (code == FC_READ_COILS) ? START_BITS : START_INBITS;
  byte out = 0;
  byte omask = 1;
  while (count--) {
    word value;
    byte status = _callback(number, &value, false);
    if (value) out |= omask;
    omask <<= 1;
    if (!omask || !count) {
      sendByte(out);
      omask = 1;
      out = 0;
    }
    number++;
  }
  sendCRC();
}

void ModBus::writeBit(byte code, word address, word value) {
  if (!checkCRC()) return;
  initCRC();
  sendByte(code);

  byte status = _callback(START_BITS + address, &value, true);
  sendWord(address);
  sendWord(value);
  
  sendCRC();
}

void ModBus::writeBits(byte code, word address, word count, byte nBytes) {
  word address2 = address;
  word count2 = count;
  
  for (byte index = 0; index < nBytes; index++) {
    byte b = readByte();
    if (!_timeout) return;
    for (byte bit = 0; bit < 8; bit++) {
      if (count == 0) break;
      word value = (b & 1) ? 0xFF00 : 0;
      byte status = _callback(START_BITS + address, &value, true);
      address++;
      count--;
      count2++;
      b >>= 1;
    }
  }
  
  if (!checkCRC()) return;
  initCRC();
  sendByte(code);
  sendWord(address2);
  sendWord(count2);
  
  sendCRC();
}

void ModBus::readRegs(byte code, word address, word count) {
  if (!checkCRC()) return;
  initCRC();

  byte nBytes = count * 2;
  sendByte(code);
  sendByte(nBytes);
  
  word number = address + (code == FC_READ_HREGS) ? START_REGS : START_INREGS;
  while (count--) {
    word value;
    byte status = _callback(number, &value, false);
    byte hi = (value >> 8);
    byte lo = (value & 0xFF);
    sendByte(hi);
    sendByte(lo);
    address++;
    count--;
  }
  sendCRC();
}

void ModBus::writeReg(byte code, word address, word value) {
  if (!checkCRC()) return;
  initCRC();
  sendByte(code);

  byte status = _callback(START_REGS + address, &value, true);
  sendWord(address);
  sendWord(value);
  
  sendCRC();
}

void ModBus::writeRegs(byte code, word address, word count, byte nBytes) {
  word address2 = address;
  word count2 = count;
  
  while (count && nBytes) {
    word hi = readByte();
    byte lo = readByte();
    if (!_timeout) return;
    
    word value = (hi << 8) | lo;
    byte status = _callback(START_REGS + address, &value, true);
    
    address++;
    count--;
    nBytes -= 2;
    count2++;
  }
  
  if (!checkCRC()) return;
  initCRC();
  sendByte(code);
  sendWord(address2);
  sendWord(count2);
  
  sendCRC();
}

*/

#define CRC_WIDTH   (8 * sizeof(CRC_TYPE))
#define CRC_TOPBIT  (1 << (CRC_WIDTH - 1))

byte                NewBus::_address;
NewBus::callback_t  NewBus::_callback;
word   NewBus::_timeout;
word   NewBus::_timer;
byte * NewBus::_argv;
byte   NewBus::_argc;

CRC_TYPE   NewBus::_crc;
CRC_TYPE   NewBus::_crcTable[256];

void NewBus::setup(byte address, callback_t callback, byte *argv, byte argc, word timeout)
{
  _address = address;
  _callback = callback;
  _timeout = timeout;
  _timer = timeout;
  _argc = argc;
  _argv = argv;

  byte div = 0;
  do {
    word rem = (word)div << (CRC_WIDTH - 8);
    for (byte bit = 8; bit > 0; bit--) {
      if (rem & CRC_TOPBIT) 
        rem = (rem << 1) ^ CRC_POLYNOMIAL;
      else 
        rem = (rem << 1);
    }
    _crcTable[div] = rem;
    /*
    Serial::print(rem, Serial::HEX);
    if ((div & 0x0F) == 0x0F)
      Serial::println();
    else 
      Serial::print(", ");
    */
  } while (++div);
  Serial::println();
}

byte NewBus::readByte(bool doCRC) {
  if (!_timer) {
    return 0;
  }
  while (!Serial::pendingIn()) {
    _delay_us(10);
    _timer--;
    if (!_timer) {
      return 0;
    }
  }
  _timer = _timeout;
  byte b = Serial::getChar();
  
  if (doCRC) {
    byte idx = b ^ (_crc >> (CRC_WIDTH - 8));
    _crc = _crcTable[idx] /* ^ (_crc << 8) */ ;
  }

  return b;
}

void NewBus::sendByte(byte b) {
  Serial::putChar(b);
  
  byte idx = b ^ (_crc >> (CRC_WIDTH - 8));
  _crc = _crcTable[idx] /* ^ (_crc << 8) */ ;
}

void NewBus::poll()
{
  if (!Serial::pendingIn()) return;
  // initialize CRC
  _crc = CRC_INITIAL;

  byte prefix1 = readByte();
  if (!_timer) return;  
  if (prefix1 != BUS_PREFIX1) return;
  byte prefix2 = readByte();
  if (!_timer) return;  
  if (prefix2 != BUS_PREFIX2) return;
  byte prefix3 = readByte();
  if (!_timer) return;  
  if (prefix3 != BUS_PREFIX3) return;

  byte address = readByte();      // first byte: address
  if (!_timer) return;  

  byte cmd = readByte();          // second byte: command
  if (!_timer) return;
  byte nArgs = readByte();        // third byte: number of arguments (bytes)
  if (!_timer) return;
  
  nArgs &= 0x0F;                  // allow up to 15 bytes of arguments
  byte idx = 0;
  while (nArgs > 0) {
    byte arg = readByte();
    if (!_timer) return;
    if (idx < _argc) {
      _argv[idx] = arg;
      idx++;
    }
    nArgs--;
  }
  byte crc = readByte(false);
  if (!_timer) return;

  // check address - receive also broadcast (0xFF)
  if (address != _address && address != ADDRESS_ALL) return;
  // check CRC
  if (crc != _crc) return;

  byte nResults;
  byte status;

  switch (cmd) {
    case CMD_REBOOT:
    {
      //cli();
      wdt_enable(WDTO_60MS);
      //for(;;) {}
      nResults = 1;
      _argv[0] = WDTCSR;
      break;
    }  
    case CMD_ECHO:
    {
      nResults = nArgs;
      break;
    }
    default:
    {
      // call callback
      nResults = 0;
      status = _callback(cmd, idx, &nResults);    
      break;
    }
  }
  
  if (address == ADDRESS_ALL) return;     // in case of broadcast, do not reply

  _crc = CRC_INITIAL;
  sendByte(BUS_PREFIX1);
  sendByte(BUS_PREFIX2);
  sendByte(BUS_PREFIX3);
  sendByte(_address);
  sendByte(cmd);
  sendByte(nResults);
  for (byte idx = 0; idx < nResults; idx++) {
    sendByte(_argv[idx]);
  }
  sendByte(_crc);
}
