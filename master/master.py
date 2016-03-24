#!/usr/bin/env python

try:
    from gpiozero import DigitalOutputDevice
    from gpiozero import DigitalInputDevice
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
    #self.rstPin.on()
    #time.sleep(0.1)
    #self.rstPin.off()
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


class Master:
    def __init__(self, bus, script = None):
        self.bus = bus
        self.rpi = RPi()
        self.bomb = node.Bomb(bus)
        self.player = node.Player(bus)
        self.dimmer = node.Dimmer(bus)
        self.nodeMap = {
            'BOMB' : self.bomb, 
            'VALVE' : node.Valve(bus),
            'FLOOR' : node.Floor(bus),
            'RFID'  : node.RFID(bus),
            'KEY'   : node.Key(bus),
            'PBX'   : node.PBX(bus),
            'P2K'   : node.P2K(bus),
            'MAP'   : node.Map(bus),
            'WC'    : node.WC(bus)
        }
        self.minutes = 60
        self.seconds = 0
        self.script = script
        self.timeTable = []

    def setDone(self, address, isDone):
        if address in self.nodeMap:
            
            if isDone == False:
                logging.info("Resetting %s" % address)
            elif isDone == True:
                logging.info("Finishing %s" % address)
            
            self.nodeMap[address].getDone(isDone)

    def getStatus(self):
        response = {}
        response['status'] = "Pauze"
        response['doorsOpen'] = False

        for name in self.nodeMap:
            values = self.nodeMap[name].getAllValues()
            response[name] = values

        return response
        
    def getTime(self):
        return 60 - (self.minutes + self.seconds / 60.0)

    def timeSyncer(self):
        logging.info("Status/time updater thread started")
        while True:
            logging.debug("Updating nodes")
            for name in self.nodeMap:
                try:
                    self.nodeMap[name].update()
                except Exception as e:
                    logging.warning("Failed to update %s (%s)" % (name, str(e)))

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
            
            time.sleep(15)
        return

    def timeTicker(self):
        logging.info("Time ticker thread started")
        while True:
            if self.bomb.enabled == True:
                if self.seconds == 0:
                    logging.info("%d minutes remaining" % self.minutes)
                    if self.minutes > 0: 
                        self.minutes -= 1
                        self.seconds = 59
                else:
                    self.seconds -= 1
            time.sleep(1)
        return
    
    def startGame(self):
        self.minutes = 60
        self.seconds = 0
        self.bomb.setTime(self.minutes, self.seconds)
        self.bomb.setEnabled(True)
        self.dimmer.setDimmer1(40)
        self.dimmer.setDimmer2(25)  
        self.dimmer.setDimmer3(0)
        self.dimmer.setDimmer4(0)

    def player1Thread(self):
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
            'LedsOn'    : lambda: self.dimmer.setDimmer4(10),
            'LedsOff'   : lambda: self.dimmer.setDimmer4(0),
            'StartGame' : lambda: self.startGame()
        }
        
        timeMap = dict()
        for (timeStamp, action) in self.timeTable:
            if not timeStamp in timeMap:
                timeMap[timeStamp] = list()
            timeMap[timeStamp].append( action )
        
        timeKeys = sorted(timeMap.keys(), reverse=False)
        #logging.debug("Time cues: %s" % str(timeKeys))

        try:
            self.startGame()
        except Exception as e:
            logging.error("Unable to start the game (%s)" % str(e))

        idx = 0
        while True:
            if not self.bomb.enabled:
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
        
    def restartAll(self):
        self.rpi.resetNetwork()
        time.sleep(3.5)
        #self.player.setTrack1(0)
        #self.player.setTrack2(0)
        #self.player.setTrack3(0)
        #self.bomb.getDone(False)
        #self.dimmer.getDone(False)
        #self.dimmer.setDimmer1(100)
        #self.dimmer.setDimmer2(100)
        
    def loop(self):
        try:
            while self.script:
                line = self.script.readline()
                if not line:
                    break
                line = line.strip()
                if not line:
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
        except Exception as e:
            logging.warning("Exception while reading script (%s)" % str(e))

        logging.info("Loaded %d entries in script" % (len(self.timeTable)))

        try:
            self.restartAll()
        except Exception as e:
            logging.warning("Failed to initialize nodes (%s)" % str(e))
        
        t1 = threading.Thread(target=self.timeTicker)
        t2 = threading.Thread(target=self.timeSyncer)
        t3 = threading.Thread(target=self.player1Thread)
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
        webapp.app.run(debug=False, host='0.0.0.0', port=8082)
        
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
