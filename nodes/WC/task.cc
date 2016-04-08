#include <Common/config.h>
#include <Common/task.h>
#include <Common/pins.h>
#include <Common/audioplayer.h>

#include <util/delay.h>
#include <avr/interrupt.h>

using namespace WCConfig;


// internal tick frequency in Hz
#define TICK_FREQ       125UL
#define PWM_FREQ        120UL

// IO pins
#define PIN_FAN         5
//#define PIN_BTN         16,17,18,19
#define PIN_BTN1        16
#define PIN_BTN2        17
#define PIN_BTN3        18
#define PIN_BTN4        19
#define PIN_PIR         14
#define PIN_ZCROSS      8
#define PIN_DIM1        11
#define PIN_DIM2        10

// IO expander control pins
#define PIN_SDA         13
#define PIN_CLK         12

// audio player track selection pins (on I/O expander)
#define XPIN_MUTE       12

#define N_DEBOUNCE      10

//const byte pinButtons[] = { PIN_BTN };

WS2803S<PIN_SDA, PIN_CLK> ioExpander;
AudioPlayer player1(ioExpander, 15, 14, 13, 17, 16);

/*
Button btn2(PIN_BTN2);
Button btn3(PIN_BTN3);
Button btn4(PIN_BTN4);
Button pir(PIN_PIR);
*/

/*
InputDebouncePin<PIN_BTN1, N_DEBOUNCE, kLow, kPullup> btn1;
InputDebouncePin<PIN_BTN2, N_DEBOUNCE, kLow, kPullup> btn2;
InputDebouncePin<PIN_BTN3, N_DEBOUNCE, kLow, kPullup> btn3;
InputDebouncePin<PIN_BTN4, N_DEBOUNCE, kLow, kPullup> btn4;
*/

InputPin<PIN_BTN1, kLow, kPullup> btn1;
InputPin<PIN_BTN2, kLow, kPullup> btn2;
InputPin<PIN_BTN3, kLow, kPullup> btn3;
InputPin<PIN_BTN4, kLow, kPullup> btn4;

InputPin<PIN_PIR> pir;

OutputPin<PIN_FAN> fan;
OutputPin<PIN_DIM1> dimUV;
OutputPin<PIN_DIM2> dimLight;

enum {
  FLAG_DONE,
  FLAG_TIMEOUT
};

volatile byte gFlags;
volatile word gMillis;
volatile word gCounts[3];

byte gTaskDone;
byte gButtonState;

void fanOff();
void fanRampUp(byte delta);
void fanRampDown(byte delta);

byte taskIsDone() {
  return gTaskDone;
}

void taskRestart() {
  gTaskDone = false;
  //bit_clear(gFlags, FLAG_DONE);
  dimUV.off();
  fanOff();
  player1.stop();
}

void taskComplete() {
  // task is complete
  bit_set(gFlags, FLAG_DONE);
  player1.stop();
  fanOff();
  gTaskDone = true;  
}


byte taskCallback(byte cmd, byte nParams, byte *nResults, byte *busParams)
{
  switch (cmd) {
    case CMD_BUTTONS:
    {
      *nResults = 1;
      busParams[0] = gButtonState;
      break;
    }
  }
  return 0;
}

void fanOff() {
  bit_clear(TCCR0A, COM0B1);
  OCR0B = 0;
}

void fanRampUp(byte delta) {
  bit_set(TCCR0A, COM0B1);
  byte newPWM = OCR0B + delta;
  if (newPWM > OCR0A) newPWM = OCR0A;
  OCR0B = newPWM;
}

void fanRampDown(byte delta) {
  byte newPWM = OCR0B;
  if (newPWM > delta) newPWM -= delta;
  else {
    newPWM = 0;
    bit_clear(TCCR0A, COM0B1);
  }
  OCR0B = newPWM;
}

void setup() {
  // setup IO pins
  btn1.setup();
  btn2.setup();
  btn3.setup();
  btn4.setup();
  pir.setup();
  fan.setup();
  dimUV.setup();
  dimLight.setup();
  
  pinWrite(PIN_BTN1, HIGH);
  pinWrite(PIN_BTN2, HIGH);
  pinWrite(PIN_BTN3, HIGH);
  pinWrite(PIN_BTN4, HIGH);
  
  // Setup Timer0
  // Set Fast PWM mode, TOP = OCRA, prescaler 1024 (64us)
  // PWM period 16ms
  TIMER0_SETUP(TIMER0_FAST_PWM_A, TIMER0_PRESCALER(PWM_FREQ));
  TCCR0A |= (1 << COM0B1);
  OCR0A = TIMER0_COUNTS(PWM_FREQ) - 1;
  OCR0B = 0;
    
  //TIMER1_SETUP(TIMER1_PWM_FAST_ICR, TIMER1_PRESCALER(PPM_FREQ));
  //ICR1 = TIMER1_COUNTS(PPM_FREQ) - 1;
  
  // Setup Timer2
  TIMER2_SETUP(TIMER2_FAST_PWM_A, TIMER2_PRESCALER(TICK_FREQ));
  OCR2A = TIMER2_COUNTS(TICK_FREQ) - 1;
  bit_set(TIMSK2, TOIE2);                 // enable timer overflow interrupt
  
  ioExpander.setup();
  player1.setup();

  taskSetup(BUS_ADDRESS);
  taskRestart();
}

void loop() {
  // DO SOMETHING
  
  static bool wasDone = false;

  if (taskIsEnabled()) {  
    /*
    bool b1 = btn1.get();
    bool b2 = btn2.get();
    bool b3 = btn3.get();
    bool b4 = btn4.get();
    */
    bool b1 = (pinRead(PIN_BTN1) == LOW);
    bool b2 = (pinRead(PIN_BTN2) == LOW);
    bool b3 = (pinRead(PIN_BTN3) == LOW);
    bool b4 = (pinRead(PIN_BTN4) == LOW);
    gButtonState = 0;
    if (b1) gButtonState |= 1;
    if (b2) gButtonState |= 2;
    if (b3) gButtonState |= 4;
    if (b4) gButtonState |= 8;

    dimLight.set(b2 && !(bit_check(gFlags, FLAG_TIMEOUT)));

    if (b3) fanRampUp(2);
    else fanRampDown(4);
  
    if (pinRead(PIN_PIR) == LOW) {
      player1.play(1);
      //_delay_ms(5);
      //player1.stop();
    }
    else player1.stop();
  
    if (b1 && b2 && b3 && b4) {
      if (!wasDone) {
        taskComplete();
        wasDone = true;
      }
    }
    else wasDone = false;
    //else pinWrite(PIN_FAN, LOW); //fan.off();
  }
    
  taskLoop();
}

ISR(TIMER2_OVF_vect) {  
  if (bit_check(gFlags, FLAG_DONE)) {
    bit_clear(gFlags, FLAG_DONE);
    dimLight.off();
    dimUV.on();
    bit_set(gFlags, FLAG_TIMEOUT);
    gMillis = 15000;
  }
  
  if (bit_check(gFlags, FLAG_TIMEOUT)) {
    if (gMillis < (1000UL / TICK_FREQ)) {
      bit_clear(gFlags, FLAG_TIMEOUT);
      dimUV.off();
    }
    else {
      gMillis -= (1000UL / TICK_FREQ);      
    }
  }
  /*
  btn1.update();
  btn2.update();
  btn3.update();
  btn4.update();
  */
}

ISR(PCINT1_vect) {
  static byte lastState;
  byte state = (PINC & 0x3F);

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
