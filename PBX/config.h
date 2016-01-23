#pragma once

#define N_DIGITS          4
#define NUMBER_FINISH     "1234"
#define NUMBER1           "2332"
#define NUMBER2           "02"

// analog sense pins
#define SENSE1_PIN        0
#define SENSE2_PIN        1

// digital control pins (ring and talk)
#define RING1_PIN         5
#define RING2_PIN         7
#define TALK_PIN          8

// phone track IDs for audio player
#define TRACK_DIAL        1
#define TRACK_CALL        2
#define TRACK_BUSY        3
#define TRACKS_RESPONSE   {10,11,12,13,14}

// sense threshold voltages (in Volts)
#define THRESHOLD_OPEN    0.2
#define THRESHOLD_CLOSED  3.2

// ADC hardware definitions
#define ADC_VREF          5.0
#define ADC_COUNTS        1024
#define VOLTS_TO_COUNTS(V)   (int)(V * ADC_COUNTS / ADC_VREF + 0.5)

