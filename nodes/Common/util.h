#pragma once

#include "types.h"

#define ARRAY_SIZE(array)     (sizeof(array) / sizeof(array[0]))

#define bit_set(var, bit)       var |= (1 << (bit))
#define bit_clear(var, bit)     var &= ~(1 << (bit))
#define bit_check(var, bit)     (var & (1 << (bit)))
#define bit_mask1(bit)          (1 << (bit))
#define bit_mask2(bit1, bit2)   ((1 << bit1) | (1 << bit2))
#define bit_mask3(b1, b2, b3)   ((1 << b1) | (1 << b2) | (1 << b3))
#define bit_mask4(b1, b2, b3, b4)   ((1 << b1) | (1 << b2) | (1 << b3) | (1 << b4))

enum PinState {
  LOW, HIGH
};

enum PinMode {
  INPUT, OUTPUT
};

#define digitalWrite  pinWrite
#define digitalRead   pinRead

void pinMode(byte pin, PinMode mode);
void pinWrite(byte pin, PinState state);
PinState pinRead(byte pin);

//#define VOLTS_TO_COUNTS(V)   (int)(V * ADC_COUNTS / ADC_VREF + 0.5)

void adcReference();
word adcRead(byte pin);


#define TIMER0_SETUP(mode, prescaler) \
{ \
  TCCR0A = (mode & 0x03); \
  uint8_t mode_b = ((mode & 0x04) << 1); \
  switch(prescaler) { \
    case 0UL: TCCR0B = TIMER0_STOP | mode_b; break; \
    case 1UL: TCCR0B = TIMER0_PRE1 | mode_b; break; \
    case 8UL: TCCR0B = TIMER0_PRE8 | mode_b; break; \
    case 64UL: TCCR0B = TIMER0_PRE64 | mode_b; break; \
    case 256UL: TCCR0B = TIMER0_PRE256 | mode_b; break; \
    case 1024UL: TCCR0B = TIMER0_PRE1024 | mode_b; break; \
  } \
}

#define TIMER0_NORMAL         0
#define TIMER0_PWM_PHASE      1
#define TIMER0_CTC            2
#define TIMER0_FAST_PWM       3
#define TIMER0_PWM_PHASE_A    5
#define TIMER0_FAST_PWM_A     7

#define TIMER0_STOP       0
#define TIMER0_PRE1       (1 << CS00)
#define TIMER0_PRE8       (1 << CS01)
#define TIMER0_PRE64      ((1 << CS00) | (1 << CS01))
#define TIMER0_PRE256     (1 << CS02)
#define TIMER0_PRE1024    ((1 << CS00) | (1 << CS02))
#define TIMER0_T0_FALL    ((1 << CS01) | (1 << CS02))
#define TIMER0_T0_RISE    ((1 << CS00) | (1 << CS01) | (1 << CS02))

#define TIMER0_PRESCALER(freq)        (F_CPU/(freq) < 256 ? 1UL : (F_CPU/(8UL*freq) < 256 ? 8UL : (F_CPU/(64UL*freq) < 256 ? 64UL : \
                                      (F_CPU/(256UL*freq) < 256 ? 256UL : (F_CPU/(1024UL*freq) < 256 ? 1024UL : 0 )))))

#define TIMER0_COUNTS(freq)           (byte)(F_CPU / (TIMER0_PRESCALER(freq) * freq))


#define TIMER2_SETUP(mode, prescaler) \
{ \
  TCCR2A = (mode & 0x03); \
  uint8_t mode_b = ((mode & 0x04) << 1); \
  switch(prescaler) { \
    case 0UL: TCCR2B = TIMER2_STOP | mode_b; break; \
    case 1UL: TCCR2B = TIMER2_PRE1 | mode_b; break; \
    case 8UL: TCCR2B = TIMER2_PRE8 | mode_b; break; \
    case 32UL: TCCR2B = TIMER2_PRE32 | mode_b; break; \
    case 64UL: TCCR2B = TIMER2_PRE64 | mode_b; break; \
    case 128UL: TCCR2B = TIMER2_PRE128 | mode_b; break; \
    case 256UL: TCCR2B = TIMER2_PRE256 | mode_b; break; \
    case 1024UL: TCCR2B = TIMER2_PRE1024 | mode_b; break; \
  } \
}

#define TIMER2_NORMAL         0
#define TIMER2_PWM_PHASE      1
#define TIMER2_CTC            2
#define TIMER2_FAST_PWM       3
#define TIMER2_PWM_PHASE_A    5
#define TIMER2_FAST_PWM_A     7

#define TIMER2_STOP       0
#define TIMER2_PRE1       (1 << CS20)
#define TIMER2_PRE8       (1 << CS21)
#define TIMER2_PRE32      ((1 << CS20) | (1 << CS21))
#define TIMER2_PRE64      (1 << CS22)
#define TIMER2_PRE128     ((1 << CS20) | (1 << CS22))
#define TIMER2_PRE256     ((1 << CS21) | (1 << CS22))
#define TIMER2_PRE1024    ((1 << CS20) | (1 << CS21) | (1 << CS22))

#define TIMER2_PRESCALER(freq)        (F_CPU/(freq) < 256 ? 1UL : (F_CPU/(8UL*freq) < 256 ? 8UL : (F_CPU/(32UL*freq) < 256 ? 32UL : (F_CPU/(64UL*freq) < 256 ? 64UL : \
                                      (F_CPU/(128UL*freq) < 256 ? 128UL : (F_CPU/(256UL*freq) < 256 ? 256UL : (F_CPU/(1024UL*freq) < 256 ? 1024UL : 0 )))))))

#define TIMER2_COUNTS(freq)           (byte)(F_CPU / (TIMER2_PRESCALER(freq) * freq))






#define TIMER1_SETUP(mode, prescaler) \
{ \
  TCCR1A = (mode & 0x03); \
  uint8_t mode_b = ((mode & 0x0C) << 1); \
  switch(prescaler) { \
    case 0UL: TCCR1B = TIMER1_STOP | mode_b; break; \
    case 1UL: TCCR1B = TIMER1_PRE1 | mode_b; break; \
    case 8UL: TCCR1B = TIMER1_PRE8 | mode_b; break; \
    case 64UL: TCCR1B = TIMER1_PRE64 | mode_b; break; \
    case 256UL: TCCR1B = TIMER1_PRE256 | mode_b; break; \
    case 1024UL: TCCR1B = TIMER1_PRE1024 | mode_b; break; \
  } \
}

#define TIMER1_NORMAL     0
#define TIMER1_PWM_P8     1
#define TIMER1_PWM_P9     2
#define TIMER1_PWM_P10    3
#define TIMER1_CTC_OCR    4
#define TIMER1_PWM_F8     5
#define TIMER1_PWM_F9     6
#define TIMER1_PWM_F10    7
#define TIMER1_PWM_PF_ICR   8
#define TIMER1_PWM_PF_OCR   9
#define TIMER1_PWM_P_ICR    10
#define TIMER1_PWM_P_OCR    11
#define TIMER1_CTC_ICR      12
#define TIMER1_PWM_FAST_ICR   14
#define TIMER1_PWM_FAST_OCR   15

#define TIMER1_STOP       0
#define TIMER1_PRE1       1
#define TIMER1_PRE8       2
#define TIMER1_PRE64      3
#define TIMER1_PRE256     4
#define TIMER1_PRE1024    5
#define TIMER1_T1_FALL    6
#define TIMER1_T1_RISE    7

#define TIMER1_PRESCALER(freq)        (F_CPU/(freq) < 65536 ? 1UL : (F_CPU/(8UL*freq) < 65536 ? 8UL : (F_CPU/(64UL*freq) < 65536 ? 64UL : \
                                      (F_CPU/(256UL*freq) < 65536 ? 256UL : (F_CPU/(1024UL*freq) < 65536 ? 1024UL : 0 )))))

#define TIMER1_COUNTS(freq)           (word)(F_CPU / (TIMER1_PRESCALER(freq) * freq))

/*
void timer2Setup(word frequency) {
  long div32 = F_CPU / frequency;
  if ((div32 >> 16) == 0) {
    word div16 = divisor;
    
  }
  // Setup Timer2
  // Set CTC mode, TOP = OCRA, prescaler 1024
  // Overflow 125Hz (8ms), overflow interrupt enabled
  TCCR2A = (1 << WGM21) | (1 << WGM20);
  TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20) | (1 << WGM22);
  TIMSK2 = (1 << TOIE2); 
  OCR2A = (byte)(F_CPU / (1024UL * TICK_FREQ)) - 1;
}
*/
