#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#include "Common/util.h"
#include "line.h"
#include "user.h"


AudioPlayer player1(3, 4, 5);
PLineConfig config1 = { 
  .apinSense = SENSE1_PIN, .pinRing = RING1_PIN, 
  .trackDial = TRACK_DIAL, .trackCall = TRACK_CALL, .trackBusy = TRACK_BUSY 
};
PLineConfig config2 = { 
  .apinSense = SENSE2_PIN, .pinRing = RING2_PIN, 
  .trackDial = TRACK_DIAL, .trackCall = TRACK_CALL, .trackBusy = TRACK_BUSY 
};
PLine line1(player1, config1);
PLine line2(player1, config2);

PUser user1(line1);
PUser user2(line2);
VUser user3(NUMBER_FINISH);
//Operator oper(user1, user2, user3);

void setup()
{
  ADCSRA = (1 << ADEN) | (1 << ADPS2);      // prescaler 16

  player1.setup();
  
  sei();
}

void loop()
{
  user1.tick();
  _delay_ms(1);
}

int main()
{
  setup();
  while (true) {
    loop();
  }
  return 0;
}
