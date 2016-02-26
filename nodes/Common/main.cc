#include <avr/interrupt.h>

void setup();
void loop();

int main()
{
  setup();
  sei();
  
  while (true) {
    loop();
  }
  return 0;
}
