#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#include <stdio.h>

#include "config.h"
#include <Common/modbus.h>
#include <Common/config.h>
#include <Common/util.h>
#include <Common/ws2803s.h>
#include <Common/serial.h>

#include "line.h"
#include "user.h"

using namespace PBXConfig;

WS2803S ioExpander(PIN_SDA, PIN_CLK);
AudioPlayer player1(ioExpander, XPIN_TRSEL0, XPIN_TRSEL1, XPIN_TRSEL2);

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

Serial serial;
NewBus bus;
byte busParams[BUS_NPARAMS];

byte busCallback(byte cmd, byte nParams, byte *nResults)
{
  switch (cmd) {
    case CMD_INIT:
    {
      break;      
    }
    
    case CMD_DONE:
    {
      break;      
    }
    
    default:
    break;
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
  
  //ADCSRA = (1 << ADEN) | (1 << ADPS2);      // prescaler 16
  ADCSRA = bit_mask2(ADPS2, ADPS0);
  bit_set(ADCSRA, ADEN);
  adcReference();
  
  serial.setup(BUS_SPEED, PIN_TXE, PIN_RXD);
  serial.enable();  
  bus.setup(BUS_ADDRESS, &busCallback, busParams, BUS_NPARAMS);
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
    pinWrite(PIN_TALK, HIGH);
  }
  else {
    pinWrite(PIN_TALK, LOW);
  }
  
  bus.poll();
}

extern "C" void __cxa_pure_virtual() { while (1); }
