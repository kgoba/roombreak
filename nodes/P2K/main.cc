#include <Common/util.h>
#include <Common/i2c.h>

#include <util/delay.h>

class I2C {
public:
  
private:
};

const byte sdaPin1 = 12;
const byte clkPin1 = 13;

//Serial serial;

void setup() {
  I2CSetup(I2C_RATE(100000));
  
  /*
  serial.setup(19200, 1, 2);
  serial.enable();
  _delay_ms(1000);
  serial.putChar('x');
  */
}

void loop() {
  _delay_ms(1000);
  
  _delay_ms(1000);
}

int main()
{
  setup();
  
  while (true) {
    loop();
  }
  return 0;
}
