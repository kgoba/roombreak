#pragma once

#define N_DIGITS          4
#define NUMBER_SOLVE1     "1234"
#define NUMBER_SOLVE2     "5996"
#define NUMBER_01         "01"
#define NUMBER_02         "02"
#define NUMBER_03         "03"
#define NUMBER_ADMIN      "04"

// analog sense pins
#define PIN_SENSE1        7
#define PIN_SENSE2        0

// digital control pins (ring and talk)
#define PIN_RING1         5
#define PIN_RING2         6
#define PIN_TALK          9

// IO expander control pins
#define PIN_SDA           13
#define PIN_CLK           12

// audio player track selection pins (on I/O expander)
#define XPIN_TRSEL0  15
#define XPIN_TRSEL1  14
#define XPIN_TRSEL2  13
#define XPIN_TRSEL3  17
#define XPIN_TRSEL4  16

//#define PLAYER1_PINS      {15, 14, 13, 17, 16}
//#define PLAYER2_PINS      {9, 10, 11, 13, 7}
//#define PLAYER3_PINS      {4, 5, 6, 2, 3}
//#define MUTE_PIN          12

// track IDs for audio player
#define TRACK_DIAL        1
#define TRACK_CALL        2
#define TRACK_BUSY        3
#define TRACK_SOLVED      99
#define TRACKS_RESPONSE   {10,11,12,13,14}

// sense threshold voltages (in Volts)
#define THRESHOLD_OPEN    0.4
#define THRESHOLD_CLOSED  1.2

// ADC hardware definitions
#define ADC_VREF          5.0
#define ADC_COUNTS        1024
#define VOLTS_TO_COUNTS(V)   (int)(V * ADC_COUNTS / ADC_VREF + 0.5)
