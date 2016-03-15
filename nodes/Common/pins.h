#include <avr/io.h>

#include "util.h"
#include "types.h"

template<int pin>
class OutPin {
public:
  static void setup() {
    if (pin < 8) {
      bit_set(DDRB, pin); 
    }
  }
  static void set(bool on) {
    if (pin < 8) {
      if (on) bit_set(PORTB, pin);
      else bit_clear(PORTB, pin);
    }
  }
};

template<int pin>
class PWMPin : public OutPin<pin> {
public:
  static void setup(uint32_t frequency = 0) {
    OutPin<pin>::setup();
    if (pin == 5 || pin == 6) {
      setFrequency(frequency);
    }
  }
  
  static void setFrequency(uint32_t freq, uint8_t mode = TIMER0_CTC) {
    stop();
    if (freq == 0) return;

    OCR0A = TIMER0_COUNTS(freq) - 1; //((31250 + freq / 2) / freq) - 1;
    
    TIMER0_SETUP(mode, TIMER0_PRESCALER(freq));
  }
  
  static void stop() {
    TCCR0A = TIMER0_STOP;
  }
  
  static void set(bool on) {
    volatile uint8_t *reg;
    uint8_t bit;

    switch (pin) {
      case 5: bit = COM0B0; break;
      case 6: bit = COM0A0; break;
      default: return;
    }
    switch (pin) {
      case 5:
      case 6: reg = &TCCR0A; break;
      default: return;
    }
    if (on) bit_set(*reg, bit);
    else bit_clear(*reg, bit);
  }
};
