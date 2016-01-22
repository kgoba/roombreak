#include <inttypes.h>
#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <util/delay.h>

// 0 for normal operation, 1 for debug (blink)
#define DEBUG             0

// Timer period in microseconds
// Should correspond to DOUBLE the bitrate (half-period)
#define TIMER_PERIOD_US   256

#define TIMER_PRESCALER   16
#define TIMER_COUNT       ( (8L * TIMER_PERIOD_US) / TIMER_PRESCALER)

// LED (output) pin
#define LED_CONFIG      DDRB |= (1 << PB2)
#define LED_ON          PORTB |= (1 << PB2)
#define LED_OFF         PORTB &= ~(1 << PB2)
#define LED_TOGGLE      PORTB ^= (1 << PB2)

// programming switch pin (pullup)
#define SWITCH_CONFIG   PORTB |= (1 << PB3)
#define SWITCH_CHECK    (PINB & (1 << PB3))

// Arduino compatibility
typedef uint8_t byte;   
typedef uint16_t word;

// Global variables
volatile byte rfid_payload[11];
volatile byte rfid_payload_ready;    // 1 - payload is received, 0 - ready to receive

byte rfid_stored[11];

void blink_led(byte count)
{
  byte i;
  for (i = 0; i < count; i++) {
    LED_ON;
    _delay_ms(250);
    LED_OFF;
    _delay_ms(250);
  }  
}

void setup() {
  cli();           // disable all interrupts

  // Setup pins
  LED_CONFIG;     // LED (output)
  SWITCH_CONFIG;  // Programming Switch
    
  // Disable system clock prescaler
  CLKPR = (1 << CLKPCE);
  CLKPR = 0;
  
  // Setup Timer1
  TCCR1 = (1 << PWM1A);   // OCR1C = TOP
  TCCR1 |= (1 << CS12) | (1 << CS10); // prescaler /16
  OCR1C = TIMER_COUNT;    // set TOP
  TIMSK = (1 << TOIE1);   // enable overflow interrupt
    
  // Setup PCINT on PB4
	PCMSK  |= (1<<PCINT4);  // configure on DEMOD pin
	GIMSK  |= (1<<PCIE);    // enable PCINT interrupt

  sei();             // enable all interrupts
  
  // restore stored RFID tag
  eeprom_read_block((void *)rfid_stored, (const void *)0, sizeof(rfid_stored));

  if (DEBUG) blink_led(3);
}

void store_tag()
{
  byte i;
  for (i = 0; i < 11; i++) 
    rfid_stored[i] = rfid_payload[i];  
  eeprom_update_block((const void *)rfid_stored, (void *)0, sizeof(rfid_stored));
}

byte compare_tag()
{
  byte is_good = 1;
  byte i;
  for (i = 0; i < 11; i++) 
    if (rfid_stored[i] != rfid_payload[i]) {
      is_good = 0;
      break;
    }
  return is_good;
}

void loop()
{
  if (!rfid_payload_ready) {
    LED_OFF;
    return;
  }

  if (!SWITCH_CHECK) {
    // programming switch on
    store_tag();
    if (DEBUG) blink_led(5);
    rfid_payload_ready = 0; // reset ready flag
  }
  else {
    // compare to stored tag
    byte match = compare_tag();
    rfid_payload_ready = 0;     // reset tag early
    
    if (match) {
      LED_ON;
      _delay_ms(250);
    }
    else {
      if (DEBUG) {
        LED_ON;
        _delay_ms(100);
      }
      LED_OFF;
      _delay_ms(250);
    }
  }
}

int main()
{
    setup();
    for (;;) loop();
}

// state machine that processes each received bit
// 0, 1 - received bit
// 2 - error (reset machine)
void em4100_machine(uint8_t bit)
{
  enum { EM4100_HEADER, EM4100_PAYLOAD };
  static uint8_t state = EM4100_HEADER;
  static uint8_t bit_count = 0;
  static uint8_t payload_idx;
  static uint8_t bit_idx;
  
  if (bit == 2) {
    state = EM4100_HEADER;
    bit_count = 0;
    return;
  }
  
  switch (state) {
  case EM4100_HEADER:
    if (bit == 1) bit_count++;
    else bit_count = 0;
    
    if (bit_count == 9 && !rfid_payload_ready) {
      state = EM4100_PAYLOAD;
      payload_idx = 0;
      bit_idx = 0;
      uint8_t i;
      for (i = 0; i < 11; i++)
        rfid_payload[i] = 0;
    }
    break;
  case EM4100_PAYLOAD:
    rfid_payload[payload_idx] <<= 1;
    rfid_payload[payload_idx] |= bit;
    bit_idx++;
    if (bit_idx == 5) {       // store only 5 received bits in each array byte
      bit_idx = 0;
      payload_idx++;
    }
    // Increment our bit counter and check for completion
    bit_count++;
    if (bit_count == 64) {
      bit_count = 0;
      state = EM4100_HEADER;
      rfid_payload_ready = 1;   // Set flag to indicate payload availability
    }
    break;
  }
}

/////////////////////////////////////////////////////////
// Interrupt routines

enum {CHANGE_UP, CHANGE_DOWN, CHANGE_NONE};

volatile uint8_t bit_change;    // used to communicate between interrupt routines

ISR(PCINT0_vect) {
  //static uint8_t last = 0;
  if (bit_is_set(PINB, PB4))
  {
    bit_change = CHANGE_UP;
  }
  else {
    bit_change = CHANGE_DOWN;
  }
  // set Timer1 midway
  TCNT1 = TIMER_COUNT / 2;
}

ISR(TIMER1_OVF_vect) {
  static uint16_t timerCounter = 0;
  static uint8_t phase = 0;

  if (!phase) {
    switch (bit_change) {
      case CHANGE_NONE:
        // we were expecting a change
        phase = !phase;
        em4100_machine(2);
        break;
      case CHANGE_UP:
        // received 1
        em4100_machine(1);
        break;
      case CHANGE_DOWN:
        // received 0
        em4100_machine(0);
        break;
    }
  }

  phase = !phase;
  bit_change = CHANGE_NONE;
  // if (phase) LED_ON; else LED_OFF;

  /*
  timerCounter++;
  if (timerCounter == 2000) {
    //LED_ON;    
  }
  else if (timerCounter == 4000) {
    //LED_OFF;
    timerCounter = 0;
  }
  */
}
