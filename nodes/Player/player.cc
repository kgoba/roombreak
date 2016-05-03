#include <Common/config.h>
#include <Common/task.h>
#include <Common/audioplayer.h>

#include <util/delay.h>
#include <avr/interrupt.h>

using namespace PlayerConfig;

// IO expander control pins
#define PIN_SDA           13
#define PIN_CLK           12

// audio player track selection pins (on I/O expander)
#define XPIN_MUTE 12

WS2803S<PIN_SDA, PIN_CLK> ioExpander;
AudioPlayer player1(ioExpander, 15, 14, 13, 17, 16);
AudioPlayer player2(ioExpander, 9, 10, 11, 8, 7);
AudioPlayer player3(ioExpander, 4, 5, 6, 2, 3);


// internal timing frequency in Hz
#define TICK_FREQ       125

enum {
  FLAG_DONE,
  FLAG_KABOOM
};

volatile byte gSecondsKaboom;
volatile word gMillis;
volatile byte gFlags;

byte gKaboomTrack;
byte gTrack1;
byte gTrack2;
byte gTrack3;

void taskRestart() {
  gTrack1 = 0;
  gTrack2 = 0;
  gTrack3 = 0;
}

void taskComplete() {
}

byte taskIsDone() {
  return 0;
}

byte taskCallback(byte cmd, byte nParams, byte *nResults, byte *busParams)
{
  switch (cmd) {
    case CMD_TRACK1:
    {
      if (nParams > 0) {
        gTrack1 = busParams[0];
      }
      busParams[0] = gTrack1;
      *nResults = 1;
      break;
    }    
    case CMD_TRACK2:
    {
      if (nParams > 0) {
        gTrack2 = busParams[0];
      }
      busParams[0] = gTrack2;
      *nResults = 1;
      break;
    }   
    case CMD_TRACK3:
    {
      if (nParams > 0) {
        gTrack3 = busParams[0];
      }
      busParams[0] = gTrack3;
      *nResults = 1;
      break;
    }
    case CMD_KABOOM:
    {
      if (nParams > 1) {
        gKaboomTrack = busParams[1];
      }
      else gKaboomTrack = 3;
      if (nParams > 0) {
        gSecondsKaboom = busParams[0];
      }
      else gSecondsKaboom = 10;
      bit_set(gFlags, FLAG_KABOOM);        
      break;
    }
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

  ioExpander.setup();
  player1.setup();
  player2.setup();
  player3.setup();
  
  taskRestart();
  taskSetup(BUS_ADDRESS);
}


void loop() {  
  // DO SOMETHING
  if (bit_check(gFlags, FLAG_KABOOM)) {
    player1.play(gKaboomTrack);
    player2.stop();
    player3.stop();
  }
  else {
    player1.play(gTrack1);
    player2.play(gTrack2);
    player3.play(gTrack3);    
  }

  taskLoop();
}

ISR(TIMER2_OVF_vect) {
  gMillis += 1000 / TICK_FREQ;
  if (gMillis >= 1000) {
    gMillis -= 1000;
    
    if (bit_check(gFlags, FLAG_KABOOM)) {
      if (gSecondsKaboom > 0) {
        gSecondsKaboom--;
      }
      else {
        bit_clear(gFlags, FLAG_KABOOM);
      }
    }
  }
}
