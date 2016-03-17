#include <Common/config.h>
#include <Common/serial.h>
#include <Common/modbus.h>
#include <Common/util.h>
#include <Common/task.h>

#include <util/delay.h>
#include <avr/interrupt.h>

using namespace ValveConfig;

// valves
// hall - a2, a1, a0
// servo - 9

// internal tick frequency in Hz
#define TICK_FREQ       125UL
#define PPM_FREQ        100UL

#define PIN_PWM         9

// (19, 18), (17, 16), (15, 14)

#define PPM_NEUTRAL_US  1280
#define PPM_MAX_US      (PPM_NEUTRAL_US + 560)
#define PPM_MIN_US      (PPM_NEUTRAL_US - 560)

#define POSITIONS       0x0708, 0x06B0, 0x062C, 0x05B8, 0x0560, 0x04F2, 0x0490, 0x0424, 0x03B6, 0x035A, 0x02F4


const word ppmDigitUs[] = { POSITIONS };

enum {
  FLAG_DONE,
  FLAG_TIMEOUT
};

volatile byte gFlags;
volatile word gMillis;
volatile int16_t gCounts[3];

int8_t gDigit;
byte gTaskDone;

void servoOn();
void servoOff();
void servoSet(word ppm_us);

byte taskIsDone() {
  return gTaskDone;
}

void taskRestart() {
  gCounts[0] = 0;
  gCounts[1] = 0;
  gCounts[2] = 0;

  gDigit = DIGIT_START;
  gTaskDone = 0;
}

void taskComplete() {
  if (taskIsDone()) return;
  // task is complete
  gDigit = DIGIT_END;
  gTaskDone = 1;
}

byte taskCallback(byte cmd, byte nParams, byte *nResults, byte *busParams)
{
  switch (cmd) {    
    case CMD_DIGIT:
    {
      if (nParams > 0) {
        gDigit = busParams[0];
      }
      *nResults = 1;
      busParams[0] = gDigit;
      //busParams[0] = WORD_LO(ppm_us);
      //busParams[1] = WORD_HI(ppm_us);
    }
  }
  return 0;
}

void setup() {  
  // setup IO pins
  pinMode(PIN_PWM, OUTPUT);
    
  TIMER1_SETUP(TIMER1_PWM_FAST_ICR, TIMER1_PRESCALER(PPM_FREQ));
  ICR1 = TIMER1_COUNTS(PPM_FREQ) - 1;
  
  // Setup Timer2
  TIMER2_SETUP(TIMER2_FAST_PWM_A, TIMER2_PRESCALER(TICK_FREQ));
  OCR2A = TIMER2_COUNTS(TICK_FREQ) - 1;
  bit_set(TIMSK2, TOIE2);                 // enable timer overflow interrupt

  bit_set(PCICR, PCIE1);    // enable pin change on PORTC
  PCMSK1 = 0x3F;            // pins PC0-PC5

  //gState = kINIT;
  servoOn();
  servoSet(PPM_MIN_US);
  _delay_ms(1000);
  servoSet(PPM_MAX_US);
  _delay_ms(1000);
  servoSet(PPM_NEUTRAL_US);
  
  taskSetup(BUS_ADDRESS);
  taskRestart();
}

void loop() {
  // DO SOMETHING
  if (bit_check(gFlags, FLAG_TIMEOUT)) {
    bit_clear(gFlags, FLAG_TIMEOUT);
  }
  
  static byte lastWheel = 0xFF;

  if (!taskIsDone()) {
    if (gCounts[0] > COUNT_THRESHOLD) {
      if (lastWheel != 0) gDigit += VALVE1_CW;
      gCounts[0] = 0;
      lastWheel = 0;
    }
    else if (gCounts[0] < -COUNT_THRESHOLD) {
      if (lastWheel != 0) gDigit += VALVE1_CCW;
      gCounts[0] = 0;    
      lastWheel = 0;
    }
  
    if (gCounts[1] > COUNT_THRESHOLD) {
      if (lastWheel != 1) gDigit += VALVE2_CCW;
      gCounts[1] = 0;
      lastWheel = 1;
    }
    else if (gCounts[1] < -COUNT_THRESHOLD) {
      if (lastWheel != 1) gDigit += VALVE2_CW;
      gCounts[1] = 0;   
      lastWheel = 1; 
    }  

    if (gCounts[2] > COUNT_THRESHOLD) {
      if (lastWheel != 2) gDigit += VALVE3_CW;
      gCounts[2] = 0;
      lastWheel = 2;
    }
    else if (gCounts[2] < -COUNT_THRESHOLD) {
      if (lastWheel != 2) gDigit += VALVE3_CCW;
      gCounts[2] = 0;   
      lastWheel = 2; 
    }    
    if (gDigit > 10) gDigit = 0;
    if (gDigit < 0) gDigit = 0;

    servoSet(ppmDigitUs[gDigit]);

    if (gDigit == DIGIT_END) {
      taskComplete();
    }
  }
    
  taskLoop();
}

ISR(TIMER2_OVF_vect) {  
  gMillis += (1000UL / TICK_FREQ);
  //gMillis += 8;
  if (gMillis >= 3000) {
    gMillis -= 3000;
    bit_set(gFlags, FLAG_TIMEOUT);
  }
}

ISR(PCINT1_vect) {
  static byte lastState;
  byte state = (PINC & 0x3F);     // 6 bits = 3 channels x 2 bits quadrature

  byte tNow = state;
  byte tLast = lastState;
  for (byte idx = 0; idx < 3; idx++) {
    byte qNow = tNow & 0x03;
    byte qLast = tLast & 0x03;
    tNow >>= 2;
    tLast >>= 2;
    if (qNow == qLast) continue;      // no change
    
    // 00 - 01 - 11 - 10
    byte phase = qNow ^ qLast;
    int8_t dir = 0;
    switch (phase) {
      case 1: dir = (qNow == 1 || qNow == 2) ? 1 : -1; break;
      case 2: dir = (qNow == 0 || qNow == 3) ? 1 : -1; break;
      case 3: break;
      case 0: break;
    }
    gCounts[idx] += dir;
  }
  lastState = state;
}

void servoOn()
{
  pinWrite(PIN_PWM, LOW);
  bit_set(TCCR1A, COM1A1);
}

void servoOff()
{
  bit_clear(TCCR1A, COM1A1);
  pinWrite(PIN_PWM, LOW);
}

void servoSet(word ppm_us)
{
  OCR1A = ((ppm_us * PPM_FREQ) / 1000UL * TIMER1_COUNTS(PPM_FREQ)) / 1000UL;
}