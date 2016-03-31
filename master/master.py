#!/usr/bin/env python

try:
    from gpiozero import DigitalOutputDevice
    from gpiozero import DigitalInputDevice, Button
    RPI_OK = True
except:
    RPI_OK = False
    pass

import webapp
    
from signal import pause

import argparse
import logging
import threading
import bisect
import time
import sys
import os
import rs485
from bus import Bus
import node

class RPi:
  PIN_NODE_RST = 27
  PIN_START_BTN = 23
  PIN_EXIT_EN = 24

  PIN_SNAKE_EN = 2
  PIN_EASTER_EN = 3
  PIN_SNAKE_DONE = 25

  def __init__(self, onStart = None, onStop = None):
    if not RPI_OK: return
    self.rstPin = DigitalOutputDevice(self.PIN_NODE_RST)
    #self.btnPin = DigitalInputDevice(self.PIN_START_BTN)
    self.btnPin = Button(self.PIN_START_BTN)
    self.outPin = DigitalOutputDevice(self.PIN_EXIT_EN, active_high = False)
    self.snakeOnPin = DigitalOutputDevice(self.PIN_SNAKE_EN, active_high = False)
    #self.outPin.source = self.btnPin.values
    if onStart: self.btnPin.when_pressed = onStart
    if onStop: self.btnPin.when_released = onStop
    return

  def resetNetwork(self):
    if not RPI_OK: return
    self.rstPin.blink(n = 1, on_time = 0.1, off_time = 0.1, background = True)
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

  def gameEnabled(self):
    return self.btnPin.is_active


class Master:
    def __init__(self, bus, script = None):
        self.bus = bus
        self.rpi = RPi()
        #self.rpi = RPi(self.onGameStart, self.onGamePause)

        try:
            self.bomb = node.Bomb(bus)
            self.player = node.Player(bus)
            self.dimmer = node.Dimmer(bus)
            self.nodeMap = {
                'BOMB'  : self.bomb, 
                'VALVE' : node.Valve(bus),
                'FLOOR' : node.Floor(bus),
                'RFID'  : node.RFID(bus),
                'KEY'   : node.Key(bus),
                'PBX'   : node.PBX(bus),
                'P2K'   : node.P2K(bus),
                'MAP'   : node.Map(bus),
                'WC'    : node.WC(bus)
            }
        except Exception as e:
            logging.warning("Failed to instantiate node objects (%s)" % str(e))

        self.minutes = 60
        self.seconds = 0
        
        self.script = script
        self.timeTable = []
        try:
            self.readScript()
            logging.info("Loaded %d entries in script" % (len(self.timeTable)))
        except Exception as e:
            logging.warning("Exception while reading script (%s)" % str(e))

        self.gameState = 'service'
        #self.setGameState('service')

    def setDone(self, address, isDone):
        if address in self.nodeMap:
            if isDone == False:
                logging.info("Resetting %s" % address)
            elif isDone == True:
                logging.info("Finishing %s" % address)
            
            self.nodeMap[address].getDone(isDone)

    def getStatus(self):
        response = {}
        response['status'] = self.gameState
        response['gameEnabled'] = self.rpi.gameEnabled()
        response['doorsOpen'] = False
        for name in self.nodeMap:
            values = self.nodeMap[name].getAllValues()
            response[name] = values
        return response
        
    def getTime(self):
        return 60 - (self.minutes + self.seconds / 60.0)

    def setGameState(self, newState):
        if newState == 'service':
            self.rpi.resetNetwork()
            time.sleep(3.5)

            self.minutes = 60
            self.seconds = 0
            self.bomb.setTime(self.minutes, self.seconds)
            #self.player.setTrack1(0)
            #self.player.setTrack2(0)
            #self.player.setTrack3(0)
            #self.bomb.getDone(False)
            #self.dimmer.getDone(False)
            #self.dimmer.setDimmer1(100)
            #self.dimmer.setDimmer2(100)
        elif newState == 'active':
            if self.gameState != 'pause':
                return
            self.dimmer.setDimmer1(self.lastDimmer1)
            self.dimmer.setDimmer2(self.lastDimmer2)
            self.bomb.setEnabled(True)
            pass
        elif newState == 'pause':
            if self.gameState == 'active':
                return
                self.lastDimmer1 = self.dimmer.getDimmer1()
                self.lastDimmer2 = self.dimmer.getDimmer2()
                self.bomb.setEnabled(False)
                self.dimmer.setDimmer1(100)
                self.dimmer.setDimmer2(100)
            else:
                self.lastDimmer1 = self.dimmer.setDimmer1(40)
                self.lastDimmer2 = self.dimmer.setDimmer2(25)
                self.dimmer.setDimmer3(0)
                self.dimmer.setDimmer4(0)
            pass
 
        logging.info("Entering game state \"%s\"" % newState)
        self.gameState = newState
        
    def ledsOn(self):
        self.dimmer.setDimmer4(10)
        self.dimmer.setDimmer1(25)
        self.dimmer.setDimmer2(50)
    
    def ledsOff(self):
        self.dimmer.setDimmer4(0)
        self.dimmer.setDimmer1(50)
        self.dimmer.setDimmer2(25)
              
    def timeSyncer(self):
        logging.info("Status/time updater thread started")
        ledState = [False] * 10
        ledList = ['VALVE', 'FLOOR', 'RFID', 'KEY', 'MAP', 'P2K', 'WC']
        while True:
            logging.debug("Updating nodes")
            for name in self.nodeMap:
                try:
                    self.nodeMap[name].update()
                except Exception as e:
                    logging.warning("Failed to update %s (%s)" % (name, str(e)))
            
            try:
                idx = 0
                for name in ledList:
                    ledState[idx] = self.nodeMap[name].done
                    idx += 1
                self.bomb.setLeds(ledState)
            except Exception as e:
                logging.warning("Failed to update Bomb LED state (%s)" % str(e))

            logging.debug("Syncing time")
            try:
                t = self.bomb.getTime()
                if t != None:
                    (minutes, seconds) = t
                    self.minutes = minutes
                    self.seconds = seconds
                    logging.debug("Time sync %02d:%02d (was %02d:%02d)" % (minutes, seconds, self.minutes, self.seconds))
            except Exception as e:
                logging.warning("Failed to get time (%s)" % str(e))
            
            time.sleep(10)
        return

    def timeTicker(self):
        logging.info("Time ticker thread started")
        while True:
            if self.gameState == 'active' and self.bomb.enabled:
                if self.seconds == 0:
                    logging.info("%d minutes remaining" % self.minutes)
                    if self.minutes > 0: 
                        self.minutes -= 1
                        self.seconds = 59
                else:
                    self.seconds -= 1
            time.sleep(1)
        return
 
    def scriptThread(self):
        logging.info("Scheduler thread started")
        
        actions = {
            'Station'   : lambda: self.player.setTrack1(1),
            'Train'     : lambda: self.player.setTrack1(2),
            'Tick'      : lambda: self.player.setTrack1(3),
            'Explode'   : lambda: self.player.setTrack1(5),
            'Laugh'     : lambda: self.player.setTrack2(2),
            'Announce'  : lambda: self.player.setTrack2(1),
            'Radio'     : lambda: self.player.setTrack3(1),
            'SnakeOn'   : lambda: self.rpi.setSnake(True),
            'SnakeOff'  : lambda: self.rpi.setSnake(False),
            'LedsOn'    : lambda: self.ledsOn(),
            'LedsOff'   : lambda: self.ledsOff()
            #'StartGame' : lambda: self.setGameState('play')
        }
        
        timeMap = dict()
        for (timeStamp, action) in self.timeTable:
            if not timeStamp in timeMap:
                timeMap[timeStamp] = list()
            timeMap[timeStamp].append( action )
        
        timeKeys = sorted(timeMap.keys(), reverse=False)
        #logging.debug("Time cues: %s" % str(timeKeys))

        #try:
        #    self.setGameState('play')
        #except Exception as e:
        #    logging.error("Unable to start the game (%s)" % str(e))

        idx = 0
        lastDimmer1 = None
        lastDimmer2 = None

        while True:
            if self.gameState == 'service':
                idx = 0
                time.sleep(1)
                continue

            if self.gameState == 'pause':
                time.sleep(1)
                continue

            if idx >= len(timeKeys): 
                logging.info("Script execution complete")
                time.sleep(60)
                continue
            timeElapsed = self.getTime()
            if timeKeys[idx] <= timeElapsed:                
                for action in timeMap[timeKeys[idx]]:
                    logging.info("New action: %s (time %02d:%02d)" % (action, self.minutes, self.seconds))
                    try:
                        actions[action]()
                    except Exception as e:
                        logging.warning("Failed to execute action %s (%s)" % (action, str(e)))

                idx += 1
            time.sleep(1)
        return        

    def readScript(self):
        while self.script:
            line = self.script.readline()
            if not line:
                break
            line = line.strip()
            if not line or line[0] == '#':
                continue
            fields = line.split()
            if len(fields) != 2:
                logging.warning("Expected 2 fields per line in script file")
                continue
            (timeString, action) = fields
            (minString, secString) = timeString.split(':')
            timeStamp = int(minString) + int(secString) / 60.0
            self.timeTable.append((timeStamp, action))
        pass


    def loop(self):
        #try:
        #    self.restartAll()
        #except Exception as e:
        #    logging.warning("Failed to initialize nodes (%s)" % str(e))
        
        t1 = threading.Thread(target=self.timeTicker)
        t2 = threading.Thread(target=self.timeSyncer)
        t3 = threading.Thread(target=self.scriptThread)
        t1.daemon = True
        t2.daemon = True
        t3.daemon = True
        t1.start()
        t2.start()
        t3.start()

        webapp.app.config['MASTER'] = self
        webapp.app.logger.setLevel(logging.WARNING)
        log = logging.getLogger('werkzeug')
        if log: log.setLevel(logging.WARNING)
        webapp.startServer()
        #webapp.app.run(debug=False, host='0.0.0.0', port=8088)
        
        while True:
            time.sleep(5)

        return

def readConfig(values):
    home = os.path.expanduser("~")    
    try:
        file = open(os.path.join(home, '.roombreak'), 'r')
    except:
        file = None
    if not file:
        return values
    
    for line in file:
        line = line.rstrip()
        if not line or line[0] == '#': continue
        (key, val) = line.split()
        values[key] = val
    
    file.close()
    return values
    
def main(args):
  readConfig(vars(args))
  if args.debug:
    level = logging.DEBUG
  else:
    level = logging.INFO
  logging.basicConfig(level = level)
  logging.debug(args)

  ser = rs485.RS485(args.port, args.baudrate, timeout = 0.2, writeTimeout = 0.2)
  bus = Bus(ser)

  script = None
  try:
      if args.script:
          script = open(args.script)
  except Exception as e:
      logging.warning("Unable to open script file (%s)" % str(e))
  master = Master(bus, script)
  master.loop()
  return 0

if __name__ == "__main__":
  parser = argparse.ArgumentParser(description = 'Metro@roombreak master scheduler')

  parser.add_argument('-p', '--port', help = 'Serial port device')
  parser.add_argument('-b', '--baudrate', help = 'Serial baudrate (default 19200)', type = int, default = 19200)
  parser.add_argument('-d', '--debug', help = 'Debug', action = 'store_true', default = False)
  parser.add_argument('-s', '--script', help = 'Script')

  args = parser.parse_args(sys.argv[1:])
  
  main(args)
