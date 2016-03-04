#include <Common/util.h>
#include <Common/ws2803s.h>
#include <Common/serial.h>
#include <Common/config.h>

#include <util/delay.h>

Serial serial;

void setAll(byte duty);

void setup() {
  /*
  serial.setup(19200, 1, 2);
  serial.enable();
  _delay_ms(1000);
  serial.putChar('x');
  */
  serial.setup(19200, PIN_TXE, PIN_RXD);
  serial.enable();

  //driver1.setup();
  //driver2.setup();

  //setAll(255);
}

void loop() {
  /*
  static byte pwm[] = {0, 1, 2, 3, 4, 6, 8, 11, 16, 22, 32, 45, 64, 90, 128, 180, 255};
  static byte i = 0;

  setAll(pwm[i]);
  _delay_ms(1000);
  
  i++;
  if (i >= sizeof(pwm)/sizeof(pwm[0])) i = 0;
  */
  
  //setAll(255);
  _delay_ms(2000);
  //*/
}
