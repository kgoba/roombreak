#pragma once

#define N_DIGITS          4
#define NUMBER_FINISH     "1234"
#define NUMBER1           "2332"
#define NUMBER2           "02"

// analog sense pins
#define SENSE1_PIN        7
#define SENSE2_PIN        0

// digital control pins (ring and talk)
#define RING1_PIN         5
#define RING2_PIN         6
#define TALK_PIN          9

// IO expander control pins
#define IO_SDA            13
#define IO_CLK            12

// audio player track selection pins
#define PLAYER1_PINS      {15, 14, 13, 17, 16}
#define PLAYER2_PINS      {9, 10, 11, 13, 7}
#define PLAYER3_PINS      {4, 5, 6, 2, 3}
#define MUTE_PIN          12

// track IDs for audio player
#define TRACK_DIAL        1
#define TRACK_CALL        2
#define TRACK_BUSY        3
#define TRACKS_RESPONSE   {10,11,12,13,14}

// sense threshold voltages (in Volts)
#define THRESHOLD_OPEN    0.2
#define THRESHOLD_CLOSED  1.2

// ADC hardware definitions
#define ADC_VREF          5.0
#define ADC_COUNTS        1024
#define VOLTS_TO_COUNTS(V)   (int)(V * ADC_COUNTS / ADC_VREF + 0.5)
