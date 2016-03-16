#include <avr/io.h>

#include "util.h"
#include "types.h"

template<int pin>
class IOPin {
public:
  static void setup() {
    if (!isValid()) return;
  }
  static bool getInput() {
    if (!isValid()) return false;
    return bit_check(*regPIN(), bit());
  }
  static void setOutput(bool high) {
    if (!isValid()) return;
    if (high) bit_set(*regPORT(), bit());
    else bit_clear(*regPORT(), bit());
  }
  static bool getOutput() {
    if (!isValid()) return false;
    return bit_check(*regPORT(), bit());
  }
    
protected:
  static bool isValid() {
    return (pin <= 22);
  }
  static uint8_t bit() {
    if (pin < 8) return pin;
    if (pin < 16) return pin - 8;
    if (pin < 24) return pin - 16;
    return 0;
  }
  static volatile uint8_t* regDDR() {
    if (pin < 8) return &DDRD;
    if (pin < 16) return &DDRB;
    if (pin < 24) return &DDRC;
    return 0;
  }
  static volatile uint8_t* regPORT() {
    if (pin < 8) return &PORTD;
    if (pin < 16) return &PORTB;
    if (pin < 24) return &PORTC;
    return 0;
  }
  static volatile uint8_t* regPIN() {
    if (pin < 8) return &PIND;
    if (pin < 16) return &PINB;
    if (pin < 24) return &PINC;
    return 0;
  }
  static volatile uint8_t* regPCMSK() {
    if (pin < 8) return &PCMSK2;
    if (pin < 16) return &PCMSK0;
    if (pin < 24) return &PCMSK1;
    return 0;    
  }
  static uint8_t pcIntBit() {
    if (pin < 8) return PCIE2;
    if (pin < 16) return PCIE0;
    if (pin < 24) return PCIE1;    
  }    
};

/*

Example:

InputPin<2, true> pin;      // active low (default high)
pin.setup(true);      // enable pullup (no pullup by default)
if (pin.get()) doSomething();

 */

typedef enum { kLow = 0, kHigh = 1 } LogicLevel;
typedef enum { kNoPullup = 0, kPullup = 1 } PullupMode;

template<int pin, LogicLevel activeLevel = kHigh>
class InputPin : public IOPin<pin> {
public:
        
  static void setup(PullupMode pullup = kNoPullup) {
    if (!IOPin<pin>::isValid()) return;
    bit_clear(*IOPin<pin>::regDDR(), IOPin<pin>::bit());
    if (pullup == kPullup) bit_set(*IOPin<pin>::regPORT(), IOPin<pin>::bit());
  }
  static bool get() {
    if (!IOPin<pin>::isValid()) return false;
    return (activeLevel == kLow) ^ IOPin<pin>::getInput();
  }
  static bool enablePCInt() {
    bit_set(PCICR, IOPin<pin>::pcIntBit());
    bit_set(*IOPin<pin>::regPCMSK(), IOPin<pin>::bit());
  }
  static bool disablePCInt() {
    bit_set(PCICR, IOPin<pin>::pcIntBit());
    bit_set(*IOPin<pin>::regPCMSK(), IOPin<pin>::bit());
  }
};

template<int pin, byte debounce, LogicLevel activeLevel = kHigh>
class InputDebouncePin : public InputPin<pin, activeLevel> {
public:

    static bool get() {
        return (_on && _counter == debounce) || (!_on && _counter > 0);
    }
    
    static void update() {
        bool on = InputPin<pin, activeLevel>::get();
        if (on) {
          if (_counter < debounce) _counter++;
          _on = true;
        }
        else {
          if (_counter > 0) _counter--;
          _on = false;
        }
    }

private:
    static byte _counter;
    static bool _on;
};

template<int pin, byte debounce, LogicLevel activeLevel>
byte InputDebouncePin<pin, debounce, activeLevel>::_counter = 0;

template<int pin, byte debounce, LogicLevel activeLevel>
bool InputDebouncePin<pin, debounce, activeLevel>::_on = false;

/*

Example:

OutputPin<2, true> pin;      // active low (default high)
pin.setup(true);             // initial on
doSomething();
pin.off();                  // switch off

 */

template<int pin, LogicLevel activeLevel = kHigh>
class OutputPin : public IOPin<pin> {
public:
  static void setup(bool initialOn = false) {
    if (!IOPin<pin>::isValid()) return;
    bit_set(*IOPin<pin>::regDDR(), IOPin<pin>::bit());
    set(initialOn);
  }
  static void set(bool on) {
    IOPin<pin>::setOutput(on ^ (activeLevel == kLow));
  }
  static void on() {
    set(true);
  }
  static void off() {
    set(false);
  }
  static bool get() {
    return IOPin<pin>::getOutput() ^ (activeLevel == kLow);
  }
};

template<int pin>
class PWMPin : public OutputPin<pin> {
public:
  static void setup() {
    OutputPin<pin>::setup();
  }

  static void setToggleFrequency(uint32_t freq, uint8_t mode = TIMER0_CTC) {
    TCCR0B = TIMER0_STOP;
    if (freq == 0) return;

    OCR0A = TIMER0_COUNTS(freq*2) - 1; //((31250 + freq / 2) / freq) - 1;
    
    TIMER0_SETUP(mode, TIMER0_PRESCALER(freq*2));
  }
    
  static void setFrequency(uint32_t freq, uint8_t mode = TIMER0_CTC) {
    TCCR0B = TIMER0_STOP;
    if (freq == 0) return;

    OCR0A = TIMER0_COUNTS(freq) - 1; //((31250 + freq / 2) / freq) - 1;
    
    TIMER0_SETUP(mode, TIMER0_PRESCALER(freq));
  }
  
  static void setPWMPercent(uint32_t freq, uint8_t percent, uint8_t mode = TIMER0_FAST_PWM) {
    volatile uint8_t *reg;
    switch (pin) {
      case 5: reg = &OCR0B; break;
      case 6: reg = &OCR0A; break;
      default: return;
    }
    *reg = (percent * 255 + 50) / 100;
    TIMER0_SETUP(mode, TIMER0_PRESCALER(freq));
  }
  
  static void enableToggle() {
    enable(1);
  }
  
  static void enable(uint8_t mode = 2) {
    volatile uint8_t *reg;

    switch (pin) {
      case 5: mode <<= 4; break;
      case 6: mode <<= 6; break;
      default: return;
    }
    switch (pin) {
      case 5:
      case 6: reg = &TCCR0A; break;
      default: return;
    }
    *reg = ((*reg) & 0x0F) | mode;
  }
  
  static void disable() {
    enable(0);
  }
};

template <bool phaseCorrect = false>
class PWM0Dual {
public:
  static void setup(uint32_t frequency = 0) {
    OutputPin<5>::setup();
    OutputPin<6>::setup();
    if (frequency) setFrequency(frequency);
  }
  
  static void setFrequency(uint32_t frequency) {
    uint8_t mode;
    if (phaseCorrect) mode = TIMER0_PWM_PHASE;
    else mode = TIMER0_FAST_PWM;

    if (frequency > 0) {
      TIMER0_SETUP(mode, TIMER0_PRESCALER(frequency));
    }
    else {
      TIMER0_SETUP(mode, TIMER0_STOP);    
    }
  }
  
  static void setToggle(bool channelA, bool channelB) {
    TCCR0A = (TCCR0A & 0xF0) | (channelA ? (1 << COM0A0) : 0) | (channelB ? (1 << COM0B0) : 0);
  }

  static void enable(bool inverted) {
    
  }
};

template<uint32_t frequency>
class Timer2Fixed {
public:
  void setup();
};
