#include <Common/config.h>
#include <Common/serial.h>
#include <Common/modbus.h>
#include <Common/util.h>
#include <Common/audioplayer.h>

#include <util/delay.h>
#include <avr/interrupt.h>

using namespace WCConfig;


// internal tick frequency in Hz
#define TICK_FREQ       125UL
#define PWM_FREQ        100UL

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

//const byte pinButtons[] = { PIN_BTN };

WS2803S ioExpander(PIN_SDA, PIN_CLK);
AudioPlayer player1(ioExpander, 15, 14, 13, 17, 16);

Button btn1(PIN_BTN1);
Button btn2(PIN_BTN2);
Button btn3(PIN_BTN3);
Button btn4(PIN_BTN4);
Button pir(PIN_PIR);
OutputPin fan(PIN_FAN);

enum {
  FLAG_DONE,
  FLAG_TIMEOUT
};

volatile byte gFlags;
volatile word gMillis;
volatile word gCounts[3];

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

void fanSet(byte state) {
  pinWrite(PIN_FAN, (PinState)state);
}

void setup() {
  // setup IO pins
  pinMode(PIN_FAN, OUTPUT);
  
  btn1.setup();
  btn2.setup();
  btn3.setup();
  btn4.setup();
  pir.setup();
  fan.setup();
  
  // Setup Timer0
  // Set Fast PWM mode, TOP = OCRA, prescaler 1024 (64us)
  // PWM period 16ms
  //TIMER0_SETUP(TIMER0_FAST_PWM_A, TIMER0_PRESCALER(PWM_FREQ));
  //TCCR0A |= (1 << COM0B1);
  //OCR0A = TIMER0_COUNTS(PWM_FREQ) - 1;
  //OCR0B = 0;
    
  //TIMER1_SETUP(TIMER1_PWM_FAST_ICR, TIMER1_PRESCALER(PPM_FREQ));
  //ICR1 = TIMER1_COUNTS(PPM_FREQ) - 1;
  
  // Setup Timer2
  TIMER2_SETUP(TIMER2_FAST_PWM_A, TIMER2_PRESCALER(TICK_FREQ));
  OCR2A = TIMER2_COUNTS(TICK_FREQ) - 1;
  bit_set(TIMSK2, TOIE2);                 // enable timer overflow interrupt

  //bit_set(PCICR, PCIE1);    // enable pin change on PORTC
  //PCMSK1 = 0x3F;            // pins PC0-PC5

  //gState = kINIT;
  //servoOn();
  //servoSet(PPM_NEUTRAL_US);
  
  ioExpander.setup();
  player1.setup();

  serial.setup(BUS_SPEED, PIN_TXE, PIN_RXD);
  serial.enable();  
  bus.setup(BUS_ADDRESS, &busCallback, busParams, BUS_NPARAMS);
  
  fan.on();
}

void loop() {
  // DO SOMETHING
  if (bit_check(gFlags, FLAG_TIMEOUT)) {
    bit_clear(gFlags, FLAG_TIMEOUT);
  }
  
//  if (btn1.check()) pinWrite(PIN_FAN, HIGH);
//  else pinWrite(PIN_FAN, LOW);
    
  bus.poll();
  _delay_ms(10);
}

ISR(TIMER2_OVF_vect) {
  static byte fan_on;
  
  gMillis += (1000UL / TICK_FREQ);
  //gMillis += 8;
  if (gMillis >= 100) {
    gMillis -= 100;
    bit_set(gFlags, FLAG_TIMEOUT);
    
    //fan_on = !fan_on;
    //OCR0B = fan_on;
    fan_on++;
    if (fan_on >= OCR0A) fan_on = 0;
    //pinWrite(PIN_FAN, (PinState)fan_on);
  }
  btn1.update();
  btn2.update();
  btn3.update();
  btn4.update();
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