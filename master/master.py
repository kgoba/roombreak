#!/usr/bin/env python

try:
    from gpiozero import DigitalOutputDevice
    from gpiozero import DigitalInputDevice
    RPI_OK = True
except:
    RPI_OK = False
    pass
    
from signal import pause

import argparse
import logging
import threading
import bisect
import time
import sys
import rs485
from bus import Bus
from node import Bomb, Player

class RPi:
  PIN_NODE_RST = 27
  PIN_START_BTN = 23
  PIN_EXIT_EN = 24

  PIN_SNAKE_EN = 2
  PIN_EASTER_EN = 3
  PIN_SNAKE_DONE = 25

  def __init__(self):
    if not RPI_OK: return
    self.rstPin = DigitalOutputDevice(self.PIN_NODE_RST)
    self.btnPin = DigitalInputDevice(self.PIN_START_BTN)
    self.outPin = DigitalOutputDevice(self.PIN_EXIT_EN, active_high = False)
    self.snakeOnPin = DigitalOutputDevice(self.PIN_SNAKE_EN, active_high = False)
    self.outPin.source = self.btnPin.values
    return

  def resetNetwork(self):
    if not RPI_OK: return
    self.rstPin.on()
    time.sleep(0.1)
    self.rstPin.off()
    #self.rstPin.blink(on_time = 0.1, off_time = 0.1, background = True)
    return

  def setSnake(self, on):
    if not RPI_OK: return
    if on: self.snakeOnPin.on()
    else: self.snakeOnPin.off()
    return

  def setDoors(self, on):
    if not RPI_OK: return
    if on: self.outPin.on()
    else: self.outPin.off()
    return 


class Master:
    def __init__(self, bus):
        self.bus = bus
        self.rpi = RPi()
        self.bomb = Bomb(bus)
        self.player = Player(bus)
        self.minutes = 60
        self.seconds = 0
        self.busLock = threading.Lock()

    def getTime(self):
        return 60 - (self.minutes + self.seconds / 60.0)

    def timeSyncer(self):
        while True:
            logging.info("Syncing time")
            self.busLock.acquire()
            t = self.bomb.getTime()
            self.busLock.release()
            if not t: 
                logging.warning("Failed to get time")
                return
            (minutes, seconds) = t
            logging.info("Time sync %02d:%02d (was %02d:%02d)" % (minutes, seconds, self.minutes, self.seconds))
            self.minutes = minutes
            self.seconds = seconds
            time.sleep(10)
        return

    def timeTicker(self):
        while True:
            if self.seconds == 0:
                logging.info("%d minutes remaining" % self.minutes)
                if self.minutes > 0: 
                    self.minutes -= 1
                    self.seconds = 59
            else:
                self.seconds -= 1
            time.sleep(1)
        return
        
    def player1Thread(self):
        actions = {
            'Play1'     : lambda: self.player.setTrack1(1),
            'Play2'     : lambda: self.player.setTrack1(2),
            'Play3'     : lambda: self.player.setTrack1(3),
            'Play5'     : lambda: self.player.setTrack1(5),
            'SnakeOn'   : lambda: self.rpi.setSnake(True),
            'SnakeOff'  : lambda: self.rpi.setSnake(False)
        }
        
        timeTable1 = {
             0.5: 'Play2',
             5.5: 'SnakeOn',
             6.1: 'SnakeOff',
             9.5: 'Play1',
            10.0: 'Play2',
            15.0: 'SnakeOn',
            15.5: 'SnakeOff',
            19.5: 'Play1',
            20.0: 'Play2',
            25.0: 'SnakeOn',
            25.5: 'SnakeOff',
            29.5: 'Play1',
            30.0: 'Play2',
            35.0: 'SnakeOn',
            35.5: 'SnakeOff',
            39.5: 'Play1',
            40.0: 'Play2',
            45.0: 'SnakeOn',
            45.5: 'SnakeOff',
            49.5: 'Play1',
            50.0: 'Play2',
            55.0: 'SnakeOn',
            55.5: 'SnakeOff',
            59.0: 'Play1',
            60.0: 'Play5',
            60.5: 'Play3'
        }
        
        keys1 = sorted(timeTable1.keys(), reverse=False)
        
        lastAction = None
        while True:
            action = None
            timeElapsed = self.getTime()
            idx = bisect.bisect_left(keys1, timeElapsed)
            if idx != -1:
                action = timeTable1[keys1[idx]]
            
            if action != lastAction:
                logging.info("New action: %s (time %02d:%02d)" % (action, self.minutes, self.seconds))
                self.busLock.acquire()
                actions[action]()
                self.busLock.release()
                lastAction = action

            time.sleep(5)
        return        

    def player2Thread(self):
        actionLaugh = lambda: self.player.setTrack2(2)
        actionAnnounce = lambda: self.player.setTrack2(1)
        
        startSound = false
        while True:
            #if self.time < 0.5 and not startSound:
                #actionLaugh()
            #    startSound = True

            time.sleep(5)
        return     
        
    def loop(self):
        self.bomb.setTime(60, 0)
        
        t1 = threading.Thread(target=self.timeTicker)
        t2 = threading.Thread(target=self.timeSyncer)
        t3 = threading.Thread(target=self.player1Thread)
        t1.daemon = True
        t2.daemon = True
        t3.daemon = True
        t1.start()
        t2.start()
        t3.start()
        #rpi.resetNetwork()
        #time.sleep(3)
        
        while True:
            time.sleep(5)

def main(args):
#  rpi = RPi()
  if args.debug:
    level = logging.DEBUG
  else:
    level = logging.INFO
  logging.basicConfig(level = level)
  logging.debug(args)

  ser = rs485.RS485(args.DEV, args.baudrate, timeout = 0.2, writeTimeout = 0.2)
  bus = Bus(ser)

  master = Master(bus)
  master.loop()

if __name__ == "__main__":
  parser = argparse.ArgumentParser(description = 'TinySafeBoot command-line tool')

  parser.add_argument('-p', help = 'Serial port device', metavar='DEV', dest = 'DEV')
  parser.add_argument('-b', '--baudrate', help = 'Serial baudrate (default 19200)', type = int, default = 19200)
  parser.add_argument('-n', '--node', help = 'Node identifier')
  parser.add_argument('-R', '--reset', help = 'Reboot', action = 'store_true', default = False)
  parser.add_argument('-d', '--debug', help = 'Debug', action = 'store_true', default = False)

  args = parser.parse_args(sys.argv[1:])
  
  main(args)