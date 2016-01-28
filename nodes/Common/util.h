#pragma once
#include <stdint.h>

typedef uint8_t   byte;
typedef uint16_t  word;

#define bit_set(var, bit)       var |= (1 << bit)
#define bit_clear(var, bit)     var &= ~(1 << bit)
#define bit_check(var, bit)     (var & (1 << bit))
#define bit_mask1(bit)          (1 << bit)
#define bit_mask2(bit1, bit2)   ((1 << bit1) | (1 << bit2))
#define bit_mask3(b1, b2, b3)   ((1 << b1) | (1 << b2) | (1 << b3))
#define bit_mask4(b1, b2, b3, b4)   ((1 << b1) | (1 << b2) | (1 << b3) | (1 << b4))

enum PinState {
  LOW, HIGH
};

enum PinMode {
  INPUT, OUTPUT
};

void pinMode(byte pin, PinMode mode);
void digitalWrite(byte pin, PinState state);
PinState digitalRead(byte pin);

//#define VOLTS_TO_COUNTS(V)   (int)(V * ADC_COUNTS / ADC_VREF + 0.5)

void analogReference();
word analogRead(byte pin);
