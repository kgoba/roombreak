#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#include <stdio.h>

#include "config.h"
#include <Common/config.h>
#include <Common/task.h>
#include <Common/ws2803s.h>

#include "line.h"
#include "user.h"

using namespace PBXConfig;

#define TICK_FREQ     1000

WS2803S<PIN_SDA, PIN_CLK> ioExpander;
AudioPlayer player1(ioExpander, XPIN_TRSEL0, XPIN_TRSEL1, XPIN_TRSEL2, XPIN_TRSEL3, XPIN_TRSEL4);

PLineConfig config1 = { 
  .apinSense = PIN_SENSE1, .pinRing = PIN_RING1, 
  .trackDial = TRACK_DIAL, .trackCall = TRACK_CALL, .trackBusy = TRACK_BUSY 
};
PLine line1(player1, config1);

PLineConfig config2 = { 
  .apinSense = PIN_SENSE2, .pinRing = PIN_RING2, 
  .trackDial = TRACK_DIAL, .trackCall = TRACK_CALL, .trackBusy = TRACK_BUSY 
};
PLine line2(player1, config2);

PUser user1(line1);
//PUser user2(line2);
//VUser user3(NUMBER_FINISH);
//Operator oper(user1, user2, user3);

bool gSolved1;
bool gSolved2;

void taskComplete() {
  //
}

void taskRestart() {
  //
  gSolved1 = false;
  gSolved2 = false;
}

byte taskIsDone() {
  return false;
}

byte taskCallback(byte cmd, byte nParams, byte *nResults, byte *busParams)
{
  switch (cmd) {
    case CMD_DONE1: {
      if (nParams > 0) {
        gSolved1 = busParams[0];
      }
      *nResults = 1;
      busParams[0] = gSolved1;
      break;
    }
    case CMD_DONE2: {
      if (nParams > 0) {
        gSolved2 = busParams[0];
      }
      *nResults = 1;
      busParams[0] = gSolved2;
      break;
    }
  }
  return 0;
}

void setup()
{
  pinMode(PIN_TALK, OUTPUT);

  ioExpander.setup();
  player1.setup();
  line1.setup();
  line2.setup();
  
  TIMER0_SETUP(TIMER0_FAST_PWM, TIMER0_PRESCALER(TICK_FREQ));
  //TCCR0A = (1 << WGM01) | (1 << WGM00);
  //TCCR0B = (1 << CS02) | (1 << CS00) | (1 << WGM02);
  TIMSK0 = (1 << TOIE0); 
  //OCR0A = (byte)(F_CPU / (1024UL * TICK_FREQ)) - 1;
  OCR0A = TIMER0_COUNTS(TICK_FREQ) - 1;
  
  //ADCSRA = (1 << ADEN) | (1 << ADPS2);      // prescaler 16
  ADCSRA = bit_mask2(ADPS2, ADPS0);
  bit_set(ADCSRA, ADEN);
  adcReference();
  
  taskSetup(BUS_ADDRESS);
  taskRestart();
}

void loop()
{
  static byte id = 0;
    
  _delay_ms(1);

  //serial.println("Tick");
  
  /*
  static word avg = 0;
  
  char buf[32];
  word aval = analogRead(config1.apinSense);
  avg += aval - (avg >> 6);
  
  sprintf(buf, "%d %d (%d)", aval, avg >> 6, avg);
  serial.println(buf);
  */
  
  line2.update();
  if (line2.getState() == PLine::CLOSED) {
    pinWrite(PIN_TALK, HIGH);
  }
  else {
    pinWrite(PIN_TALK, LOW);
  }
  
  taskLoop();
}

ISR(TIMER0_OVF_vect) {
  user1.tick();
}

extern "C" void __cxa_pure_virtual() { while (1); }
