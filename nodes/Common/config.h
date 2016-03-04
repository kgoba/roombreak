#pragma once

#include "util.h"

// Global hardware definitions common to all nodes

// RS-485 RX/TX driver enable pins
#define PIN_TXE         2
#define PIN_RXD         3

// RS-485 bus speed
#define BUS_SPEED       19200

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

// Broadcast address
#define ADDRESS_ALL     0xFF

// Common commands
#define CMD_INIT        0x01      // Init (reset) task
#define CMD_DONE        0x02      // Query if task is done or force it

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
  const byte BUS_NPARAMS = 4;
  
  // servo PPM time in microseconds for 2 positions
  const word PPM_CLOSED_US  = (1500 - 300);
  const word PPM_OPEN_US    = (1500 + 200);

  // time to hold servo in a position
  const word HOLD_MS        = 1000;
};

namespace FloorConfig {
  const byte BUS_ADDRESS = ADDRESS_FLOOR;
  const byte BUS_NPARAMS = 4;
  
  const word BOUNCE_US   = 2;
};

namespace BombConfig {
  const byte BUS_ADDRESS = ADDRESS_BOMB;
  const byte BUS_NPARAMS = 4;
};

namespace MapConfig {
  const byte BUS_ADDRESS = ADDRESS_MAP;
  const byte BUS_NPARAMS = 4;
};

namespace RFIDConfig { 
  const byte BUS_ADDRESS = ADDRESS_RFID;
  const byte BUS_NPARAMS = 4;
};

namespace PlayerConfig { 
  const byte BUS_ADDRESS = ADDRESS_PLAYER;
  const byte BUS_NPARAMS = 4;
};

namespace P2KConfig {
  const byte BUS_ADDRESS = ADDRESS_P2K;
  const byte BUS_NPARAMS = 4;
};

namespace PBXConfig {
  const byte BUS_ADDRESS = ADDRESS_PBX;
  const byte BUS_NPARAMS = 4;
};
