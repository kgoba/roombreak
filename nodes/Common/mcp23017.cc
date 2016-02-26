#include "mcp23017.h"
#include "i2c.h"

#define MCP23017_ADDRESS 0x20

// registers
#define MCP23017_IODIRA 0x00
#define MCP23017_IPOLA 0x02
#define MCP23017_GPINTENA 0x04
#define MCP23017_DEFVALA 0x06
#define MCP23017_INTCONA 0x08
#define MCP23017_IOCONA 0x0A
#define MCP23017_GPPUA 0x0C
#define MCP23017_INTFA 0x0E
#define MCP23017_INTCAPA 0x10
#define MCP23017_GPIOA 0x12
#define MCP23017_OLATA 0x14


#define MCP23017_IODIRB 0x01
#define MCP23017_IPOLB 0x03
#define MCP23017_GPINTENB 0x05
#define MCP23017_DEFVALB 0x07
#define MCP23017_INTCONB 0x09
#define MCP23017_IOCONB 0x0B
#define MCP23017_GPPUB 0x0D
#define MCP23017_INTFB 0x0F
#define MCP23017_INTCAPB 0x11
#define MCP23017_GPIOB 0x13
#define MCP23017_OLATB 0x15

#define MCP23017_INT_ERR 255

MCP23017::MCP23017(byte address) {
  _address = (address | MCP23017_ADDRESS) << 1;
}

void MCP23017::setup(word iomode, word pullup) {
  I2CSetup(I2C_RATE(100000));
  I2CEnable();  
  setIOMode(iomode);
  setPullup(pullup);
}

byte MCP23017::readA() {
  return readRegister(MCP23017_GPIOA);
}

byte MCP23017::readB() {
  return readRegister(MCP23017_GPIOB);  
}

word MCP23017::read() {
  byte a = readRegister(MCP23017_GPIOA);
  word b = readRegister(MCP23017_GPIOB);
  return (b << 8) | a;
}

void MCP23017::writeA(byte value) {
  writeRegister(MCP23017_GPIOA, value);
}

void MCP23017::writeB(byte value) {
  writeRegister(MCP23017_GPIOB, value);  
}

void MCP23017::write(word value) {
  byte a = value & 0xFF;
  byte b = value >> 8;
  writeRegister(MCP23017_GPIOA, a);  
  writeRegister(MCP23017_GPIOB, b);  
}

void MCP23017::setIOMode(word mode) {
  byte a = mode & 0xFF;
  byte b = mode >> 8;
	writeRegister(MCP23017_IODIRA, a);
	writeRegister(MCP23017_IODIRB, b);
}

void MCP23017::setPullup(word mode) {
  byte a = mode & 0xFF;
  byte b = mode >> 8;
	writeRegister(MCP23017_GPPUA, a);
	writeRegister(MCP23017_GPPUB, b);
}

byte MCP23017::readRegister(byte addr) {
  byte message[] = { addr };
  I2CWriteBytes(_address, message, 1, 1);
  I2CReadBytes(_address, message, 1);
  return message[0];
}

void MCP23017::writeRegister(byte addr, byte value) {
  byte message[] = { addr, value };
  I2CWriteBytes(_address, message, 2);
}
