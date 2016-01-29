#include <Common/util.h>
#include <Common/serial.h>
#include <Common/mcp23017.h>

#include <util/delay.h>

MCP23017 driver(0);

Serial serial;

void setup() {
  //I2CSetup(I2C_RATE(100000));
  
  driver.setup(0x0000);     // all as outputs
  
  serial.setup(19200, 1, 2);
  serial.enable();
  _delay_ms(1000);
  serial.println("Setup done");
}

void loop() {
  serial.println("Low");
  driver.write(0x0000);
  _delay_ms(1000);
  
  serial.println("High");
  driver.write(0xFFFF);
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
