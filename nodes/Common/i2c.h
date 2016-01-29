#pragma once

#include "types.h"

#define I2C_AUX(x)      ((( ((uint32_t)F_CPU + x/2) / x) - 16) / 2)
#define I2C_RATE(x)     (I2C_AUX(x) < 256 ? I2C_AUX(x) : (I2C_AUX(x)/4 + 0x0100))

enum {
  I2C_OK = 0,
  I2C_ERR_NO_START,
  I2C_ERR_NO_ACK
};

enum {
  I2C_W = 0,
  I2C_R
};

/*
struct I2CMessage {
  byte *data;
  byte length;
  byte type;
};
*/

void I2CSetup(word rate);
void I2CEnable();
void I2CDisable();

byte I2CWriteBytes(byte address, const byte *data, byte count, byte noStop = 0);
byte I2CReadBytes(byte address, byte *data, byte count, byte noStop = 0);
