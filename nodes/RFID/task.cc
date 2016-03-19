#include <Common/config.h>
#include <Common/task.h>

#include <util/delay.h>
#include <avr/interrupt.h>

using namespace RFIDConfig;

#define PIN_SENSE   19, 17, 15, 13, 11, 8
#define PIN_ENABLE  18, 16, 14, 12,  9, 7
#define PIN_BUZZER  5

// 6 sensors
#define SENSOR_MASK   0x1F

// internal timing frequency in Hz
#define TICK_FREQ       125

#define BUZZ_TICKS      10
#define BUZZ_TICKS_LONG      80

// sensor pin numbers
const byte kPinSense[] = { PIN_SENSE };
const byte kPinEnable[] = { PIN_ENABLE };

enum {
  FLAG_DONE,
  FLAG_BUZZ,
  FLAG_BUZZ_LONG
};

volatile byte gFlags;
volatile word gMillis;

byte gSensorState;
byte gSensorMask = SENSOR_MASK;
byte gTaskDone;

byte taskIsDone() {
  return gTaskDone;
}

void taskRestart() {
  gSensorState = 0;
  gTaskDone = false;
  //bit_clear(gFlags, FLAG_DONE);
  bit_set(gFlags, FLAG_BUZZ);
}

void taskComplete() {
  // task is complete
  gSensorState = gSensorMask;
  gTaskDone = true;
  
  bit_set(gFlags, FLAG_BUZZ_LONG);
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

void buzzerOn()
{
  bit_set(TCCR0A, COM0B1);
}

void buzzerOff()
{
  bit_clear(TCCR0A, COM0B1);
}

void setup() {
  // Setup ENABLE pins
  for (byte iPin = 0; iPin < ARRAY_SIZE(kPinEnable); iPin++) {
    pinMode(kPinEnable[iPin], OUTPUT);
  }
  
  pinMode(5, OUTPUT);
  // Setup Timer0
  // Set Fast PWM mode, TOP = OCRA, prescaler 256
  TCCR0A = (1 << WGM01) | (1 << WGM00);
  TCCR0B = (1 << CS02) | (1 << WGM02);
  OCR0A = 100 - 1;
  OCR0B = OCR0A / 2;

  // Setup Timer2
  // Set CTC mode, TOP = OCRA, prescaler 1024
  // Overflow 125Hz (8ms), overflow interrupt enabled
  TIMER2_SETUP(TIMER2_FAST_PWM_A, TIMER2_PRESCALER(TICK_FREQ));
  OCR2A = TIMER2_COUNTS(TICK_FREQ) - 1;
  TIMSK2 = (1 << TOIE2); 

  //gState = kINIT;
  
  //buzzerOn();
  //_delay_ms(1000);
  //buzzerOff();

  taskSetup(BUS_ADDRESS);
  taskRestart();
}


void loop() {  
  // DO SOMETHING

  /*
  bool found = false;
  for (byte iPin = 0; iPin < ARRAY_SIZE(kPinEnable); iPin++) {
    if (pinRead(kPinSense[iPin]) == HIGH) {
      found = true;
    }
  }
  if (found) bit_set(gFlags, FLAG_BUZZ);

  bus.poll();
  _delay_ms(50);
  */
  
  if (!taskIsDone()) {
    bool triggerSound = false;
    for (byte idx = 0; idx < ARRAY_SIZE(kPinSense); idx++) {
      bool newState = (pinRead(kPinSense[idx]) == HIGH);
      bool lastState = bit_check(gSensorState, idx);
      if (newState) {
        if (!lastState && bit_check(gSensorMask, idx)) {
          bit_set(gFlags, FLAG_BUZZ);          
        }
        bit_set(gSensorState, idx);
      }
      else {
        bit_clear(gSensorState, idx);
      }
    }
    if (gSensorState == gSensorMask) {
      gTaskDone = true;
    }
    if (taskIsDone()) {
      taskComplete();
    }
  }
  taskLoop();
}

ISR(TIMER2_OVF_vect) {
  static word buzzCounter;
  if (bit_check(gFlags, FLAG_BUZZ)) {
    bit_set(TCCR0A, COM0B1);
    buzzCounter = BUZZ_TICKS;
    bit_clear(gFlags, FLAG_BUZZ);
  }
  if (bit_check(gFlags, FLAG_BUZZ_LONG)) {
    bit_set(TCCR0A, COM0B1);
    buzzCounter = BUZZ_TICKS_LONG;
    bit_clear(gFlags, FLAG_BUZZ_LONG);
  }
  
  if (buzzCounter > 0) {
    buzzCounter--;
    if (!buzzCounter) bit_clear(TCCR0A, COM0B1);
  }
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
