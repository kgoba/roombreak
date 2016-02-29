#include <avr/interrupt.h>
#include <avr/wdt.h>

void setup();
void loop();

/*
static void reset_mcusr(void) \
  __attribute__((naked)) \
  __attribute__((section(".init3")));
static void reset_mcusr(void)
{
  //mcusr_mirror = MCUSR;
  MCUSR = 0;
  wdt_disable();
}
*/

int main()
{
  MCUSR = 0;
  wdt_disable();
  //reset_mcusr();
  
  setup();
  sei();
  
  while (true) {
    loop();
  }
  return 0;
}
