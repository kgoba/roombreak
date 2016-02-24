#include <Common/util.h>
#include <Common/ws2803s.h>
#include <Common/serial.h>

#include <util/delay.h>

const byte sdaPin1 = 12;
const byte clkPin1 = 13;

const byte sdaPin2 = 4;
const byte clkPin2 = 5;

Serial serial;

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
  serial.setup(19200, 2, 3);
  serial.enable();

  driver1.setup();
  driver2.setup();

  setAll(255);
}

void loop() {
  static byte pwm[] = {0, 1, 2, 3, 4, 6, 8, 11, 16, 22, 32, 45, 64, 90, 128, 180, 255};
  static byte i = 0;

  setAll(pwm[i]);
  _delay_ms(1000);
  
  i++;
  if (i >= sizeof(pwm)/sizeof(pwm[0])) i = 0;
  
  //setAll(255);
  //_delay_ms(2000);
  //*/
}

void setAll(byte duty) {
  for (byte i = 0; i < 3 * 18; i++) {
    driver1.set(i, duty);
  }
  for (byte j = 0; j < 2 * 18; j++) {
    driver2.set(j, duty);
  }
  driver1.update();
  driver2.update();
}

int main()
{
  setup();
  
  while (true) {
    loop();
  }
  return 0;
}
