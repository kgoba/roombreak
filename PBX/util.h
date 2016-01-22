#pragma once
#include <avr/io.h>

#include "config.h"

typedef uint8_t   byte;
typedef uint16_t  word;

enum PinState {
  LOW, HIGH
};

enum PinMode {
  INPUT, OUTPUT
};

void pinMode(byte pin, PinMode mode);
void digitalWrite(byte pin, PinState state);
PinState digitalRead(byte pin);

#define VOLTS_TO_COUNTS(V)   (int)(V * ADC_COUNTS / ADC_VREF + 0.5)

void analogReference();
word analogRead(byte pin);
