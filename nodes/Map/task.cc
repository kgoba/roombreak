#include <Common/ws2803s.h>

#include <avr/interrupt.h>
#include <string.h>

#include "task.h"
#include "keypad.h"

// LED driver pins
#define PIN_SDA             19
#define PIN_CLK             18


// 0, 1, 3, 4, 5, 6 (BUĻĻU KĀPA - DRAUDZĪBA)
// 2, 9, 10, 11, 12, 15, 7 (IMANTA-CENTRS)
// 13, 14, 16, 7 (DZINTARS-CENTRS)
// 17, 27, 7 (SARKANAIS KVADRĀTS-CENTRS)
// 22, 21, 20, 19, 18, 6 (JUGLA-DRAUDZĪBA)
// 26, 25, 24, 33, 6 (DREILIŅI-DRAUDZĪBA)
// 29, 30, 31, 32, 6 (SARKADRAUGAVA-DRAUDZĪBA)
enum {
  LED_CENTR = 7,
  LED_DRAUD = 6,

  LED_BULLU = 0,
  LED_KLEIS = 1,
  LED_LACUP = 3,
  LED_ILGUC = 4,
  LED_KIPSA = 5,

  LED_IMANT = 2,
  LED_ZOLIT = 9,
  LED_ZASUL = 10,
  LED_PLESK = 11,
  LED_AUROR = 12,
  LED_UZVAR = 15,

  LED_DZINT = 13,
  LED_STRAU = 14,
  LED_TELEC = 16,
  
  LED_SKVAD = 17,
  LED_KENGA = 27,

  LED_DREIL = 26,
  LED_PLAVN = 25,
  LED_PURVC = 24,
  LED_KIROV = 33,

  LED_JUGLA = 22,
  LED_ALFA  = 21,
  LED_TEIKA = 20,
  LED_VEF   = 19,
  LED_OSKAL = 18,
  
  LED_SDAUG = 29,
  LED_DIZEL = 30,
  LED_ENERG = 31,
  LED_PETER = 32
};

const byte route1[] = { LED_ILGUC, LED_KIPSA, LED_DRAUD };
const byte route2[] = { LED_DRAUD, LED_OSKAL, LED_VEF, LED_TEIKA };
const byte route3[] = { LED_TEIKA, LED_VEF, LED_OSKAL, LED_DRAUD, LED_CENTR };
const byte route4[] = { LED_CENTR, LED_UZVAR, LED_AUROR };

Keypad keypad; // outPins, N_KEYPAD_COL, inPins, N_KEYPAD_ROW, (char *)keyTable);
WS2803S leds(PIN_SDA, PIN_CLK, 2);

void buzzOn(word freq)
{
  OCR0A = ((31250 + freq / 2) / freq) - 1;
  bit_set(TCCR0A, COM0B0);
}

void buzzOff()
{
  bit_clear(TCCR0A, COM0B0);  
}

void processKeyTest(char key) {
  static byte current;
  
  if (key >= '0' && key <= '9') {
    current *= 10;
    current += (key - '0');
  }
  else if (key == '*') {
    leds.clear();
    if (current == 255) 
      for (byte i = 0; i < 36; i++) leds.set(i, current);
    else
      for (byte i = 0; i < 36; i++) leds.set(current, 255);
    leds.update();
    current = 0;
  }
  else if (key == '#') {
    current = 0;
  }
}

#define N_BUFFER    4
#define N_TICKETS   4

void Task::processKey(char key) {
  static char buffer[N_BUFFER];
  
  // shift buffer and push the new key
  for (byte idx = 1; idx < N_BUFFER; idx++) {
    buffer[idx - 1] = buffer[idx];
  }
  buffer[N_BUFFER - 1] = key;

  for (byte iTicket = 0; iTicket < N_TICKETS; iTicket++) {
    if (0 == memcmp(buffer, _config.tickets[iTicket].number, N_BUFFER)) {
      _state = ENTERED;
      _iEntered = iTicket;
      break;
    }
  }
  
  if (key == '*') {
    leds.clear();
    leds.update();
  }
}

void Task::setup() {
  //serial.setup(19200, 2, 3);
  leds.setup();
  keypad.setup();
  
  _config.load();
  _state = ENTERING;
}

void Task::loop() {
  static char last = 0;
  char key = keypad.getKey();
  
  if (key != 0 && key != last) {
    buzzOn(880);
    processKey(key);
  }
  else buzzOff();
  last = key;

  if (_state == ENTERED) {
    byte iMap = 4;
    byte mask = 0x01;
    for (byte iLed = 0; iLed < 36; iLed++) {
      if (_config.tickets[_iEntered].ledMap[iMap] & mask) {
        leds.set(iLed, 255);
      }
      mask <<= 1;
      if (!mask) {
        mask = 0x01;
        iMap--;
      }
    }
    _state = ENTERING;
    leds.update();
  }
}

void Config::load() {
  byte ledMap1[] = {0x00, 0x00, 0x00, 0x00, 0xFF};
  byte ledMap2[] = {0x00, 0x00, 0x00, 0xFF, 0x00};
  byte ledMap3[] = {0x00, 0x00, 0xFF, 0x00, 0x00};
  byte ledMap4[] = {0x03, 0xFF, 0x00, 0x00, 0x00};
  char number1[] = "1111";
  char number2[] = "2222";
  char number3[] = "3333";
  char number4[] = "4444";

  tickets[0].init(ledMap1, number1);
  tickets[1].init(ledMap2, number2);
  tickets[2].init(ledMap3, number3);
  tickets[3].init(ledMap4, number4);
}

ISR(TIMER2_OVF_vect) {
  /*
  if (gMillis >= 8) gMillis -= 8;
  else {
    bit_set(gFlags, FLAG_TIMEOUT);
  }
  
  if (pinRead(PIN_SWITCH) == LOW) {
    bit_set(gFlags, FLAG_BUTTON);
  }
  */
}
