#include <Common/ws2803s.h>
#include <Common/pins.h>

#include <util/delay.h>
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
  LED_KIPSA = 5,  // ->LED_DRAUD

  LED_IMANT = 2,
  LED_ZOLIT = 9,
  LED_ZASUL = 10,
  LED_PLESK = 11,
  LED_AUROR = 12,
  LED_UZVAR = 15,   // ->LED_CENTR

  LED_DZINT = 13,
  LED_STRAU = 14,
  LED_TELEC = 16,   // ->LED_CENTR
  
  LED_SKVAD = 17,
  LED_KENGA = 27,   // ->LED_CENTR

  LED_DREIL = 26,
  LED_PLAVN = 25,
  LED_PURVC = 24,
  LED_KIROV = 33,   // ->LED_DRAUD

  LED_JUGLA = 22,
  LED_ALFA  = 21,
  LED_TEIKA = 20,
  LED_VEF   = 19,
  LED_OSKAL = 18,   // ->LED_DRAUD
  
  LED_SDAUG = 29,
  LED_DIZEL = 30,
  LED_ENERG = 31,
  LED_PETER = 32    // ->LED_DRAUD
};

//const byte route1[] = { LED_ILGUC, LED_KIPSA, LED_DRAUD };
//const byte route2[] = { LED_DRAUD, LED_OSKAL, LED_VEF, LED_TEIKA };
//const byte route3[] = { LED_TEIKA, LED_VEF, LED_OSKAL, LED_DRAUD, LED_CENTR };
//const byte route4[] = { LED_CENTR, LED_UZVAR, LED_AUROR };

const byte route1[] = { LED_ZASUL, LED_PLESK, LED_AUROR, LED_UZVAR, LED_CENTR };
const byte route2[] = { LED_CENTR, LED_DRAUD, LED_KIPSA, LED_ILGUC, LED_LACUP, LED_KLEIS };
const byte route3[] = { LED_KLEIS, LED_LACUP, LED_ILGUC, LED_KIPSA, LED_DRAUD, LED_OSKAL, LED_VEF, LED_TEIKA, LED_ALFA, LED_JUGLA };
const byte route4[] = { LED_JUGLA, LED_ALFA, LED_TEIKA, LED_VEF, LED_OSKAL, LED_DRAUD, LED_CENTR, LED_UZVAR, LED_AUROR };

Keypad keypad; // outPins, N_KEYPAD_COL, inPins, N_KEYPAD_ROW, (char *)keyTable);
WS2803S<PIN_SDA, PIN_CLK, 2> leds;
PWMPin<5> pinBuzzer;        // Timer0, pin 5 (OCR0B)

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

byte Task::processKey(char key) {  
  pinBuzzer.disable();
  if (key == _lastKey) return 0;  
  _lastKey = key;
  if (key == 0) return 0;
  
  pinBuzzer.enableToggle();
  return 1;
}

void Task::setup() {
  pinBuzzer.setup();
  pinBuzzer.setToggleFrequency(880);
    
  leds.setup();
  keypad.setup();
  
  _config.load();
  
  _lastKey = 0;
}

void Task::restart() {
  for (byte iMap = 0; iMap < 5; iMap++) _ledMap[iMap] = 0;
  _iEntered = 0;
  _keyBufferSize = 0;
  leds.clear();
  leds.update();
}

void soundOK() {
  pinBuzzer.setToggleFrequency(440);
  pinBuzzer.enableToggle();
  _delay_ms(100);
  pinBuzzer.setToggleFrequency(660);
  pinBuzzer.enableToggle();
  _delay_ms(100);
  pinBuzzer.setToggleFrequency(880);
  pinBuzzer.enableToggle();
  _delay_ms(100);
  pinBuzzer.disable();
}

void soundFail() {
  pinBuzzer.setToggleFrequency(880);
  pinBuzzer.enableToggle();
  _delay_ms(100);
  pinBuzzer.disable();
  pinBuzzer.setToggleFrequency(440);
  pinBuzzer.enableToggle();
  _delay_ms(200);  
}

void Task::complete() {
  for (byte iMap = 0; iMap < 5; iMap++) _ledMap[iMap] = 0;
  
  for (byte iTicket = 0; iTicket < N_TICKETS; iTicket++) {
    showRoute(iTicket);
  }
  _iEntered = N_TICKETS;
  leds.update();
  soundOK();
  soundOK();
}

bool Task::isFinished() {
  return _iEntered == N_TICKETS;
}

void Task::showRoute(byte iTicket) {
  byte iMap = 0;
  byte mask = 0x01;
  for (byte iLed = 0; iLed < 36; iLed++) {
    if (_config.tickets[iTicket].ledMap[iMap] & mask) {
      leds.set(iLed, 255);
    }
    else if (_ledMap[iMap] & mask) {
      leds.set(iLed, 4);
    }
    mask <<= 1;
    if (!mask) {
      mask = 0x01;
      iMap++;
    }
  }
  for (byte iMap = 0; iMap < 5; iMap++) _ledMap[iMap] |= _config.tickets[iTicket].ledMap[iMap];  
}

void Task::loop() {
  if (isFinished()) return;
  
  char key = keypad.getKey();
  if (processKey(key) == 0) return;
    
  if (_keyBufferSize < N_BUFFER) {
    _keyBuffer[_keyBufferSize] = key;
    _keyBufferSize++;
  }
  
  bool found = false;
  byte iTicket = _iEntered;

  //for (iTicket = 0; iTicket < N_TICKETS; iTicket++) {
    if (0 == memcmp(_keyBuffer, _config.tickets[iTicket].number, _keyBufferSize)) {
      found = true;
      //break;
    }
    //}
  
  if (found) {
    if (_keyBufferSize == N_BUFFER) {
      showRoute(iTicket);
      leds.update();
      _keyBufferSize = 0;
      _iEntered++;
      if (isFinished()) {
        complete();
      }
    }
  }
  else {
    _keyBufferSize = 0;
    soundFail();
    restart();
  }

  /*
  if (found) {
    _keyBufferSize = 0;        

    _iEntered |= (1 << iTicket);
    showRoute(iTicket);
    leds.update();
  }
  else {
    //restart();
  }
  */
}

void Task::tick() {
  
}

void setArrayBit(byte *array, byte idx) {
  byte nByte = idx / 8;
  byte nBit = idx % 8;
  array[nByte] |= (1 << nBit);
}

void Config::load() {
  byte ledMap1[] = {0x00, 0x00, 0x00, 0x00, 0x00};
  byte ledMap2[] = {0x00, 0x00, 0x00, 0x00, 0x00};
  byte ledMap3[] = {0x00, 0x00, 0x00, 0x00, 0x00};
  byte ledMap4[] = {0x00, 0x00, 0x00, 0x00, 0x00};
  
  char number1[] = "AB1764";
  char number2[] = "AC8498";
  char number3[] = "BA2873";
  char number4[] = "AA2699";

  for (byte idx1 = 0; idx1 < ARRAY_SIZE(route1); idx1++) setArrayBit(ledMap1, route1[idx1]);
  for (byte idx2 = 0; idx2 < ARRAY_SIZE(route2); idx2++) setArrayBit(ledMap2, route2[idx2]);
  for (byte idx3 = 0; idx3 < ARRAY_SIZE(route3); idx3++) setArrayBit(ledMap3, route3[idx3]);
  for (byte idx4 = 0; idx4 < ARRAY_SIZE(route4); idx4++) setArrayBit(ledMap4, route4[idx4]);

  tickets[0].init(ledMap1, number1);
  tickets[1].init(ledMap2, number2);
  tickets[2].init(ledMap3, number3);
  tickets[3].init(ledMap4, number4);
}
