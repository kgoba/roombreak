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
    self.outPin = DigitalOutputDevice(self.PIN_EXIT_EN, active_high = False)
    self.snakeOnPin = DigitalOutputDevice(self.PIN_SNAKE_EN, active_high = False)
    self.outPin.source = self.btnPin.values
    return

  def resetNetwork(self):
    self.rstPin.on()
    time.sleep(0.1)
    self.rstPin.off()
    #self.rstPin.blink(on_time = 0.1, off_time = 0.1, background = True)
    return

  def setSnake(self, on):
    if on: self.snakeOnPin.on()
    else: self.snakeOnPin.off()
    return

  def setDoors(self, on):
    if on: self.outPin.on()
    else: self.outPin.off()
    return 

def main(args):
  rpi = RPi()

  if args.reset:
    rpi.resetNetwork()

  while True:
    for cmd in args.command:
    if cmd == 'reset':
      rpi.resetNetwork()
    if cmd == 'wait':
      pause()
    if cmd == 'pause':
      time.sleep(1)
    if cmd == 'open':
      rpi.setDoors(True)
    if cmd == 'close':
      rpi.setDoors(False)
    if cmd == 'snake':
      rpi.setSnake(True)
    if cmd == 'uart':
      ser = rs485.RS485(args.port, args.baudrate, timeout = 0.2, writeTimeout = 0.2)
      while True:
        ser.write([0xAA])
        time.sleep(0.020)
    
    if not args.loop:
      break

  return


if __name__ == "__main__":
  parser = argparse.ArgumentParser(description = 'TinySafeBoot command-line tool')

  parser.add_argument('-p', '--port', help = 'Serial port device')
  parser.add_argument('-b', '--baudrate', help = 'Serial baudrate (default 19200)', type = int, default = 19200)
  parser.add_argument('-n', '--node', help = 'Node identifier')
  parser.add_argument('-R', '--reset', help = 'Reboot', action = 'store_true', default = False)
  parser.add_argument('-d', '--debug', help = 'Debug', action = 'store_true', default = False)
  parser.add_argument('-l', '--loop', help = 'Loop', action = 'store_true', default = False)
  parser.add_argument('command', nargs='+')

  args = parser.parse_args(sys.argv[1:])
  
  main(args)
