#include <Common/ws2803s.h>

#include "task.h"
#include "keypad.h"

#define KEY_NONE      0xFFFF

Serial serial;

byte outPins[] = KEYPAD_COL_PINS;
byte inPins[] = KEYPAD_ROW_PINS;
char keyTable[][4] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

Keypad keypad(outPins, N_KEYPAD_COL, inPins, N_KEYPAD_ROW, (char *)keyTable);
WS2803S leds(LED_SDA, LED_CLK, 2);

void processKey(word keycode) {
  static byte current;
  
  byte row = keycode & 0xFF;
  byte col = keycode >> 8;
  char key = keyTable[row][col];
  
  if (key >= '0' && key <= '9') {
    current *= 10;
    current += (key - '0');
  }
  else if (key == '*') {
    for (byte i = 0; i < 36; i++) leds.set(i, current);
    leds.update();
    current = 0;
  }
  else if (key == '#') {
    current = 0;
  }
  //char buf[100];
  //sprintf(buf, "%c: %04x", key, keycode);
  //serial.println(buf);
  serial.putChar(key);
}

void Task::setup() {
  leds.setup();
  keypad.setup();
  
  _config.load();
}

void Task::loop() {
  static word last = KEY_NONE;
  word keycode = keypad.scan();
  
  if (keycode != last && keycode != KEY_NONE) {
    processKey(keycode);
  }
  last = keycode;
}
