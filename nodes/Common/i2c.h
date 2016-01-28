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

void I2CSetup(word rate);
void I2CEnable();
void I2CDisable();
