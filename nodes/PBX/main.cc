#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#include <stdio.h>

#include "config.h"
#include <Common/util.h>
#include <Common/ws2803s.h>
#include <Common/serial.h>

#include "line.h"
#include "user.h"

WS2803S ioExpander(IO_SDA, IO_CLK);
AudioPlayer player1(ioExpander, 15, 14, 13);
//AudioPlayer player2(ioExpander, 9, 10, 11);
//AudioPlayer player3(ioExpander, 4, 5, 6);

PLineConfig config1 = { 
  .apinSense = SENSE1_PIN, .pinRing = RING1_PIN, 
  .trackDial = TRACK_DIAL, .trackCall = TRACK_CALL, .trackBusy = TRACK_BUSY 
};
PLine line1(player1, config1);

PLineConfig config2 = { 
  .apinSense = SENSE2_PIN, .pinRing = RING2_PIN, 
  .trackDial = TRACK_DIAL, .trackCall = TRACK_CALL, .trackBusy = TRACK_BUSY 
};
PLine line2(player1, config2);

PUser user1(line1);
//PUser user2(line2);
//VUser user3(NUMBER_FINISH);
//Operator oper(user1, user2, user3);

Serial serial;

void setup()
{
  ADCSRA = (1 << ADEN) | (1 << ADPS2);      // prescaler 16

  sei();
  ioExpander.setup();
  player1.setup();
  line1.setup();
  line2.setup();
  serial.setup(19200, 2, 3);
  serial.enable();
  //player2.setup();
  //player3.setup();
  //serial.println("Setup done");
  
  ADCSRA = bit_mask2(ADPS2, ADPS0);
  bit_set(ADCSRA, ADEN);
  analogReference();
  
  pinMode(TALK_PIN, OUTPUT);
}

void loop()
{
  static byte id = 0;
    
  _delay_ms(1);

  user1.tick();
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
    digitalWrite(TALK_PIN, HIGH);
  }
  else {
    digitalWrite(TALK_PIN, LOW);
  }
}

extern "C" void __cxa_pure_virtual() { while (1); }
