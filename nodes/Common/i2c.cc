#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/twi.h>

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
  while (!bit_check(TWCR, TWINT)) {}
  return TW_STATUS;
}

void I2CStop() {
  TWCR = bit_mask3(TWINT, TWSTO, TWEN);
}

byte I2CStatus() {
  while (!bit_check(TWCR, TWINT));
  return TW_STATUS;
}

byte I2CSend(byte data) {
  TWDR = data;
  TWCR = bit_mask2(TWINT, TWEN);
  while (!bit_check(TWCR, TWINT));
  return TW_STATUS;
}

byte I2CRead(byte nak) {
  TWCR = nak ? bit_mask2(TWINT, TWEN) : bit_mask3(TWINT, TWEN, TWEA);
  while (!bit_check(TWCR, TWINT));  
  return TWDR;
}

byte I2CWriteBytes(byte address, const byte *data, byte count, byte noStop) {
  byte status;
  byte result = I2C_OK;

  byte nTries = 3;
  while (nTries > 0) 
  {
    status = I2CStart();
    if (status != TW_START && status != TW_REP_START) {
      nTries--;
      if (nTries == 0)
        return I2C_ERR_NO_START;
      continue;
    }

    status = I2CSend(address | TW_WRITE);
    if (status != TW_MT_SLA_ACK) {
      nTries--;
      continue;
    }
    while (count--) {
      status = I2CSend(*data);
      if (status != TW_MT_DATA_ACK) {
        result = I2C_ERR_NO_ACK;
        break;
      }
      data++;
    }
    if (status != TW_MT_DATA_ACK) {
      result = I2C_ERR_NO_ACK;
      continue;
    }
    result = I2C_OK;
    break;
  }
  if (!noStop) {
    I2CStop();    
  }
  return result;
}

byte I2CReadBytes(byte address, byte *data, byte count, byte noStop) {
  byte status;
  byte result = I2C_OK;

  status = I2CStart();
  if (status != TW_START && status != TW_REP_START) {
    return I2C_ERR_NO_START;
  }

  status = I2CSend(address | TW_READ);
  if (status != TW_MR_SLA_ACK) {
    result = I2C_ERR_NO_ACK;
  }
  else
  while (count--) {
    *data = I2CRead(count == 0);
    status = TW_STATUS;
    if (status != TW_MR_DATA_ACK) {
      break;
    }
    data++;
  }

  if (!noStop) {
    I2CStop();    
  }
  return result;
}

/*

byte I2CTransfer(byte address, I2CMessage *messages, byte nMessages) {
  byte status;
  byte result = I2C_OK;
  
  while (nMessages--) {
    status = I2CStart();
    is (status != TW_START && status != TW_REP_START) {
      result = I2C_ERR_NO_START;
      break;
    }
    byte *ptr = messages->data;
    byte count = messages->length;
    if (messages->type == I2C_R) {
      status = I2CSend(address | TW_READ);
      if (status != TW_MR_SLA_ACK) {
        result = I2C_ERR_NO_ACK;
        break;
      }
      while (count--) {
        *ptr = I2CRead(count == 0);
        status = TW_STATUS;
        if (status != TW_MR_DATA_ACK) {
          break;
        }
        ptr++;
      }
    }
    else {
      status = I2CSend(address | TW_WRITE);
      if (status != TW_MT_SLA_ACK) {
        result = I2C_ERR_NO_ACK;
        break;
      }
      while (count--) {
        status = I2CSend(*ptr);
        if (status != TW_MT_DATA_ACK) {
          result = I2C_ERR_NO_ACK;
          break;
        }
        ptr++;
      }
    }
    if (result != I2C_OK) break;      
  
    messages++;
  }
  
  if (result != I2C_ERR_NO_START) {
    I2CStop();    
  }
  return result;
}
*/
