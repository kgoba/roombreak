#include <Common/util.h>

#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>

#include "task.h"

/*

Hardware:
- Keypad (4x4)
- 1 izeja buzzerim
- 2xWS2803 LED (36 gab.)

Uzdevums:

Savadīt 4 biļetes pareizā secībā. 
Biļetes nr. ir 4 simboli, ko ievada ar keypad. Nospiežot pogu, ir skaņas indikācija.
Pēc katras pareizās biļetes ievadīšanas atskan skaņas indikācija un iedegas noteikti LED.
Ievadot jebko nepareizu, atgriežas sākuma stāvoklī.
Pēc 4 pareizu biļešu ievadīšanas atskan skaņas indikācija un uzdevums ir atrisināts.

3 skaņas indikācijas:
1) poga nospiesta
2) biļetes numurs ievadīts
3) uzdevums atrisināts

Uzdevuma konfigurācija:
- 1. biļetes maršruta LED saraksts (5 baiti)
- 2. biļetes maršruta LED saraksts (5 baiti)
- 3. biļetes maršruta LED saraksts (5 baiti)
- 4. biļetes maršruta LED saraksts (5 baiti)
- 1. biļetes numurs (4 baiti)
- 2. biļetes numurs (4 baiti)
- 3. biļetes numurs (4 baiti)
- 4. biļetes numurs (4 baiti)

 */

Task task;

void setup() {
  Serial::setup(38400, 2, 3);
  Serial::enable();
  
  task.setup();

  sei();

  Serial::println("Setup done");
}

void loop() {
  task.loop();
  _delay_ms(50);
}

int main()
{
  setup();
  
  while (true) {
    loop();
  }
  return 0;
}
