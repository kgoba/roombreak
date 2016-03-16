#include <Common/config.h>
#include <Common/serial.h>
#include <Common/modbus.h>
#include <Common/util.h>
#include <Common/task.h>
#include <Common/audioplayer.h>

#include <util/delay.h>
#include <avr/interrupt.h>

using namespace FloorConfig;

//#define PIN_SENSE       19, 18, 17, 16, 15, 14, 7, 8, 10, 11
#define PIN_SENSE       19, 18, 17, 16, 15, 14

// IO expander control pins
#define PIN_SDA           13
#define PIN_CLK           12

// audio player track selection pins (on I/O expander)
#define XPIN_TRSEL0  15
#define XPIN_TRSEL1  14
#define XPIN_TRSEL2  13
#define XPIN_TRSEL3  17
#define XPIN_TRSEL4  16

// 6 sensors
#define SENSOR_MASK   0x3F

WS2803S ioExpander(PIN_SDA, PIN_CLK);
AudioPlayer player1(ioExpander, XPIN_TRSEL0, XPIN_TRSEL1, XPIN_TRSEL2, XPIN_TRSEL3, XPIN_TRSEL4);

const byte pinSense[] = { PIN_SENSE };

// internal timing frequency in Hz
#define TICK_FREQ       125

// sensor pin numbers
const byte kPinSense[] = { PIN_SENSE };

// audio player track selection pin numbers starting from LSB
//const byte PIN_PLAYER[] = {};

enum {
  FLAG_DONE
};

byte gSensorState;
byte gSensorMask = SENSOR_MASK;
byte gSensorDone;

volatile byte gFlags;
volatile word gMillis;

void servoOn();
void servoOff();
void servoSet(word ppm_us);

byte taskIsDone() {
  return gSensorDone == gSensorMask;
}

void taskRestart() {
  gSensorDone = 0;
  gSensorState = 0;
  //bit_clear(gFlags, FLAG_DONE);
}

void taskComplete() {
  if (taskIsDone()) return;
  // task is complete
  gSensorDone = gSensorMask;
  //bit_set(gFlags, FLAG_DONE);
}

byte taskCallback(byte cmd, byte nParams, byte *nResults, byte *busParams)
{
  switch (cmd) {    
    case CMD_SENSORSTATE:
    {
      busParams[0] = gSensorState;
      *nResults = 1;
      break;
    }

    case CMD_SENSORMASK:
    {
      if (nParams > 0) {
        gSensorMask = busParams[0];
      }
      busParams[0] = gSensorMask;
      *nResults = 1;
      break;
    }
  }
  return 0;
}

void setup() {
  for (byte idx = 0; idx < ARRAY_SIZE(pinSense); idx++)
    pinWrite(pinSense[idx], HIGH);
  
  // Setup Timer2
  // Set CTC mode, TOP = OCRA, prescaler 1024
  // Overflow 125Hz (8ms), overflow interrupt enabled
  TCCR2A = (1 << WGM21) | (1 << WGM20);
  TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20) | (1 << WGM22);
  TIMSK2 = (1 << TOIE2); 
  OCR2A = (byte)(F_CPU / (1024UL * TICK_FREQ)) - 1;

  ioExpander.setup();
  player1.setup();
  
  taskSetup(BUS_ADDRESS);
  taskRestart();
}

void loop() {  
  // DO SOMETHING
  bool triggerSound = false;
  for (byte idx = 0; idx < ARRAY_SIZE(pinSense); idx++) {
    bool newState = (pinRead(pinSense[idx]) == LOW);
    bool lastState = bit_check(gSensorState, idx);
    if (newState && !lastState && bit_check(gSensorMask, idx) && !bit_check(gSensorDone, idx)) {
      triggerSound = true;
      bit_set(gSensorDone, idx);
    }
    if (newState) {
      bit_set(gSensorState, idx);
    }
    else {
      bit_clear(gSensorState, idx);
    }
  }
  if (triggerSound) {
    player1.play(1);
  }
  else {
    player1.stop();    
  }
  if (gSensorDone == gSensorMask) {
    taskComplete();
  }

  taskLoop();
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
