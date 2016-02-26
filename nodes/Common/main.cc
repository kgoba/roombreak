#include <avr/interrupt.h>
#include <avr/wdt.h>

void setup();
void loop();

int main()
{
  MCUSR = 0;
  wdt_disable();
  
  setup();
  sei();
  
  while (true) {
    loop();
  }
  return 0;
}
