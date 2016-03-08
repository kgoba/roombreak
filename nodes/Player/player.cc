#include <Common/config.h>
#include <Common/serial.h>
#include <Common/modbus.h>
#include <Common/util.h>
#include <Common/audioplayer.h>

#include <util/delay.h>
#include <avr/interrupt.h>

using namespace PlayerConfig;

// IO expander control pins
#define PIN_SDA           13
#define PIN_CLK           12

// audio player track selection pins (on I/O expander)
#define XPIN_MUTE 12

WS2803S ioExpander(PIN_SDA, PIN_CLK);
AudioPlayer player1(ioExpander, 15, 14, 13, 17, 16);
AudioPlayer player2(ioExpander, 9, 10, 11, 8, 7);
AudioPlayer player3(ioExpander, 4, 5, 6, 2, 3);


// internal timing frequency in Hz
#define TICK_FREQ       125


enum {
  FLAG_DONE
};


volatile byte gFlags;
volatile word gMillis;
byte gState;

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


void setup() {
  // Setup Timer2
  // Set CTC mode, TOP = OCRA, prescaler 1024
  // Overflow 125Hz (8ms), overflow interrupt enabled
  TCCR2A = (1 << WGM21) | (1 << WGM20);
  TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20) | (1 << WGM22);
  TIMSK2 = (1 << TOIE2); 
  OCR2A = (byte)(F_CPU / (1024UL * TICK_FREQ)) - 1;

  serial.setup(BUS_SPEED, PIN_TXE, PIN_RXD);
  serial.enable();  
  bus.setup(BUS_ADDRESS, &busCallback, busParams, BUS_NPARAMS);

  ioExpander.setup();
  player1.setup();
  player2.setup();
  player3.setup();
}


void loop() {  
  // DO SOMETHING
  player1.play(1);
  player2.play(1);
  player3.play(1);

  bus.poll();
  _delay_ms(100);
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
