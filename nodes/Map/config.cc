#include "config.h"

void Config::load() {
  byte ledMap1[] = {0x00, 0x00, 0x00, 0x00, 0xFF};
  byte ledMap2[] = {0x00, 0x00, 0x00, 0xFF, 0x00};
  byte ledMap3[] = {0x00, 0x00, 0xFF, 0x00, 0x00};
  byte ledMap4[] = {0x00, 0xFF, 0x00, 0x00, 0x00};
  char number1[] = "1111";
  char number2[] = "2222";
  char number3[] = "3333";
  char number4[] = "4444";

  tickets[0].init(ledMap1, number1);
  tickets[1].init(ledMap2, number2);
  tickets[2].init(ledMap3, number3);
  tickets[3].init(ledMap4, number4);
}
