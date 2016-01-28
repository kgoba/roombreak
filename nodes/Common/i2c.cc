#include <avr/interrupt.h>
#include <avr/io.h>

#include "util.h"
#include "i2c.h"

void I2CSetup(word rate) {
  TWBR = rate & 0xFF;
  TWSR = (rate >> 8) & 0x03;
}

void I2CEnable() {
  bit_set(TWCR, TWEN);
}

void I2CDisable() {
  bit_clear(TWCR, TWEN);
}

byte I2CStart() {
  TWCR = bit_mask3(TWINT, TWSTA, TWEN);
  while (!bit_check(TWCR, TWINT));
  return TWSR & 0xF8;
}

void I2CStop() {
  TWCR = bit_mask3(TWINT, TWSTO, TWEN);
}

byte I2CStatus() {
  while (!bit_check(TWCR, TWINT));
  return TWSR & 0xF8;
}

void I2CSend(byte data) {
  TWDR = data;
  while (!bit_check(TWCR, TWINT));
  return TWSR & 0xF8;
}

struct I2CMessage {
  byte *data;
  byte length;
  byte type;
};

byte I2CTransfer(byte address, I2CMessage *messages, byte nMessages) {
  byte status;
  byte adr2 = (address << 1);
  
  while (nMessages--) {
    
    status = I2CStart();
    is (status != 0x08) {
      return I2C_ERR_NO_START;
    }
    status = I2CSend(messages->type == I2C_W ? (ad2) : (ad2 | 0x01));
    if (status != ) {
      return I2C_ERR_NO_ACK;
    }
    byte *ptr = messages->data;
    byte count = messages->length;
    while (count--) {
      status = I2CSend(*ptr);
      if (status != MT_DATA_ACK) {
        return I2C_ERR_NO_ACK;
      }
      ptr++;
    }
  
    messages++;
  }
  I2CStop();
  return I2C_OK;
}