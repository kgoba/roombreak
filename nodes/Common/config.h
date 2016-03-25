#pragma once

#include "util.h"

// Global hardware definitions common to all nodes

// RS-485 RX/TX driver enable pins
#define PIN_TXE         2
#define PIN_RXD         3

// Loop delay in ms (roughly period)
#define LOOP_DELAY_MS   20

// RS-485 bus speed
#define BUS_SPEED       19200

// Maximum number of bytes in the parameter field
#define BUS_NPARAMS     4

// Individual node addresses
#define ADDRESS_PBX     17
#define ADDRESS_BOMB    18
#define ADDRESS_P2K     19
#define ADDRESS_RFID    20
#define ADDRESS_KEY     21
#define ADDRESS_FLOOR   22
#define ADDRESS_MAP     23
#define ADDRESS_VALVE   24
#define ADDRESS_WC      25
#define ADDRESS_PLAYER  26
#define ADDRESS_DIMMER  27

// Broadcast address
#define ADDRESS_ALL     0xFF      // Use this to broadcast to all nodes (no reply is sent)

// Common commands
#define CMD_INIT        0x01      // Write 1 to reset task, reads 0 while initializing, 1 when initialized successfully
#define CMD_DONE        0x02      // Query if task is done or force it by writing 1

#define CMD_ECHO        0x00      // Echo back the same parameters
#define CMD_REBOOT      0x7F      // Software reset to bootloader

// Bus CRC polynomial
#define CRC_POLYNOMIAL  0x21
#define CRC_INITIAL     0xFF

// Bus message prefix
#define BUS_PREFIX1     0xAF
#define BUS_PREFIX2     0x6A
#define BUS_PREFIX3     0xDE


namespace KeyConfig {
  const byte BUS_ADDRESS = ADDRESS_KEY;
  
  // servo PPM time in microseconds for 2 positions
  const word PPM_CLOSED_US  = (1500 - 300);
  const word PPM_OPEN_US    = (1500 + 200);

  // time to hold servo in a position
  const word HOLD_MS        = 1000;
};

namespace FloorConfig {
  const byte BUS_ADDRESS = ADDRESS_FLOOR;
  
  const word BOUNCE_US   = 2;
  
  const byte CMD_SENSORSTATE = 0x10;
  const byte CMD_SENSORMASK  = 0x11;
  const byte CMD_SENSORDONE  = 0x12;
};

namespace BombConfig {
  const byte BUS_ADDRESS = ADDRESS_BOMB;
  
  const byte CMD_TIME        = 0x10;
  const byte CMD_LEDS        = 0x11;
  const byte CMD_ENABLE      = 0x12;
};

namespace MapConfig {
  const byte BUS_ADDRESS = ADDRESS_MAP;
  
  const byte CMD_TICKET1 = 0x10;
  const byte CMD_TICKET2 = 0x11;
  const byte CMD_TICKET3 = 0x12;
  const byte CMD_TICKET4 = 0x13;

  const byte CMD_ROUTE1  = 0x20;
  const byte CMD_ROUTE2  = 0x21;
  const byte CMD_ROUTE3  = 0x22;
  const byte CMD_ROUTE4  = 0x23;
  
  const byte CMD_LASTKEY = 0x30;
};

namespace RFIDConfig { 
  const byte BUS_ADDRESS = ADDRESS_RFID;
  
  const byte CMD_SENSORSTATE = 0x10;
  const byte CMD_SENSORMASK  = 0x11;
  const byte CMD_SENSORDONE  = 0x12;
};

namespace ValveConfig {
  const byte BUS_ADDRESS = ADDRESS_VALVE;
  
  const byte CMD_DIGIT   = 0x10;

  const int16_t COUNT_THRESHOLD  = 100;

  const int8_t VALVE1_CW      = -5;
  const int8_t VALVE1_CCW     = +7;

  const int8_t VALVE2_CW      = +6;
  const int8_t VALVE2_CCW     = -3;

  const int8_t VALVE3_CW      = +5;
  const int8_t VALVE3_CCW     = -6;

  const byte DIGIT_START = 0;
  const byte DIGIT_END   = 9;
};

namespace PlayerConfig { 
  const byte BUS_ADDRESS = ADDRESS_PLAYER;
  
  const byte CMD_TRACK1  = 0x10;
  const byte CMD_TRACK2  = 0x11;
  const byte CMD_TRACK3  = 0x12;
};

namespace P2KConfig {
  const byte BUS_ADDRESS = ADDRESS_P2K;
  
  const byte CMD_COUNT  = 0x10;
};

namespace PBXConfig {
  const byte BUS_ADDRESS = ADDRESS_PBX;
  
  const byte CMD_DONE1  = 0x10;
  const byte CMD_DONE2  = 0x11;
};

namespace WCConfig {
  const byte BUS_ADDRESS = ADDRESS_WC;
  
  const byte CMD_BUTTONS = 0x10;
};

namespace DimmerConfig {
  const byte BUS_ADDRESS = ADDRESS_DIMMER;
  
  const byte CMD_DIMMER1 = 0x10;
  const byte CMD_DIMMER2 = 0x11;
  const byte CMD_DIMMER3 = 0x12;
  const byte CMD_DIMMER4 = 0x13;
};
