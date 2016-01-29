#include <Common/util.h>
#include <Common/ws2308s.h>

#include <util/delay.h>

const byte sdaPin1 = 12;
const byte clkPin1 = 13;

const byte sdaPin2 = 1;
const byte clkPin2 = 2;

//Serial serial;

WS2803S driver1(sdaPin1, clkPin1, 3);
WS2803S driver2(sdaPin2, clkPin2, 2);

void setAll(byte duty);

void setup() {
  /*
  serial.setup(19200, 1, 2);
  serial.enable();
  _delay_ms(1000);
  serial.putChar('x');
  */
  driver1.setup();
  driver2.setup();
}

void loop() {
  setAll(0);
  _delay_ms(1000);
  
  setAll(255);
  _delay_ms(1000);
}

void setAll(byte duty) {
  for (byte i = 0; i < 3 * 18; i++) {
    driver1.set(i, duty);
  }
  for (byte j = 0; j < 2 * 18; j++) {
    driver2.set(j, duty);
  }
}

int main()
{
  setup();
  
  while (true) {
    loop();
  }
  return 0;
}
