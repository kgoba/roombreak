#!/usr/bin/env python
from gpiozero import DigitalOutputDevice
from gpiozero import DigitalInputDevice
from signal import pause

import argparse
import logging
import time
import sys

import rs485

class RPi:
  PIN_NODE_RST = 27
  PIN_START_BTN = 23
  PIN_EXIT_EN = 24

  PIN_SNAKE_EN = 2
  PIN_EASTER_EN = 3
  PIN_SNAKE_DONE = 25

  def __init__(self):
    self.rstPin = DigitalOutputDevice(self.PIN_NODE_RST)
    self.btnPin = DigitalInputDevice(self.PIN_START_BTN)
    self.outPin = DigitalOutputDevice(self.PIN_EXIT_EN)
    self.snakeOnPin = DigitalOutputDevice(self.PIN_SNAKE_EN, active_high = False)
    return

  def resetNetwork(self):
    self.rstPin.on()
    time.sleep(0.1)
    self.rstPin.off()
    #self.rstPin.blink(on_time = 0.1, off_time = 0.1, background = False)
    return

  def setSnake(self, on):
    if on: self.snakeOnPin.on()
    else: self.snakeOnPin.off()
    return

def main(args):
  rpi = RPi()

  if args.reset:
    rpi.resetNetwork()

  for cmd in args.command:
    if cmd == 'reset':
      rpi.resetNetwork()

  #outPin.source = btnPin.values
  #pause()

if __name__ == "__main__":
  parser = argparse.ArgumentParser(description = 'TinySafeBoot command-line tool')

  parser.add_argument('-p', help = 'Serial port device', metavar='DEV', dest = 'DEV')
  parser.add_argument('-b', '--baudrate', help = 'Serial baudrate (default 19200)', type = int, default = 19200)
  parser.add_argument('-n', '--node', help = 'Node identifier')
  parser.add_argument('-R', '--reset', help = 'Reboot', action = 'store_true', default = False)
  parser.add_argument('-d', '--debug', help = 'Debug', action = 'store_true', default = False)
  parser.add_argument('command', nargs='+')

  args = parser.parse_args(sys.argv[1:])
  
  main(args)
