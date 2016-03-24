#!/usr/bin/env python

import argparse
import logging
import rs485
import struct
import time
import sys
import os

from threading import Lock

CRC_POLYNOMIAL  = 0x21
CRC_INITIAL     = 0xFF

class CRC:
    def __init__(self, width, polynomial, initial):
        table = []
        topbit = 1 << (width - 1)
        mask = (1 << width) - 1
        for div in range(256):
            rem = div << (width - 8)
            for bit in range(8):
                if rem & topbit:
                    rem = (rem << 1) ^ polynomial
                else:
                    rem = (rem << 1)
            table.append(rem & mask)
        self.table = table
        self.poly = polynomial
        self.width = width
        self.init = initial
        return
        
    def compute(self, data):
        rem = self.init
        mask = (1 << self.width) - 1
        for d in data:
            idx = d ^ (rem >> (self.width - 8))
            rem = self.table[idx] ^ (rem << 8)
            rem = rem & mask
        return rem

def prettyFormat(data, format = "hex"):
    if format == "hex":
        str = " ".join('%02X' % b for b in data)
    elif format == "bin":
        str = " ".join("{0:b}".format(b) for b in data)
    else:
        str = "?"
    return '[' + str + '] (%d b)' % len(data)

class Bus:
    N_RETRIES       = 5
    CMD_ECHO        = 0x00
    CMD_REBOOT      = 0x7F
    PREFIX          = [0xAF, 0x6A, 0xDE]
    
    ADDRESS_MAP = {
        'PBX'   : 17, 
        'BOMB'  : 18, 
        'P2K'   : 19, 
        'RFID'  : 20,
        'KEY'   : 21,
        'FLOOR' : 22,
        'MAP'   : 23,
        'VALVE' : 24,
        'WC'    : 25,
        'PLAYER': 26,
        'DIMMER': 27
    }
    
    def __init__(self, serial, crc = CRC(8, CRC_POLYNOMIAL, CRC_INITIAL)):
        self.lock = Lock()
        self.ser = serial
        self.crc = crc
        
    def makePacket(self, address, cmd, params = None, request = True):
        if not request: cmd = cmd | 0x80
        if params: nParams = len(params)
        else: nParams = 0
        data = bytearray(self.PREFIX + [address, cmd, nParams])
        if params: data += params
        data.append(self.crc.compute(data))
        return data

    def parsePacket(self, packet):
        if not packet or len(packet) < len(self.PREFIX) + 2:
            return None
        if list(packet[0:len(self.PREFIX)]) != self.PREFIX:
            return None
        crc = self.crc.compute(packet[:-1])
        if crc != packet[-1]:
            return None         
        
        nParams = packet[len(self.PREFIX) + 2]
        params = packet[len(self.PREFIX) + 3: -1]

        if len(params) != nParams:
            return None

        address = packet[len(self.PREFIX)]
        cmdReq = packet[len(self.PREFIX) + 1]
        request = (cmdReq & 0x80) == 0
        cmd = cmdReq & 0x7F
        return (address, cmd, params, request)
        
       
    def check(self, data, recv):
        if not recv or len(recv) < len(self.PREFIX) + 2:
            return False
        addr = data[len(self.PREFIX)]
        if recv[len(self.PREFIX)] != addr:
            return False
        cmd = data[len(self.PREFIX) + 1]
        if recv[len(self.PREFIX) + 1] != (cmd | 0x80):
            return False
        crc = self.crc.compute(recv[:-1])
        return crc == recv[-1]

    def getAddresses(self):
        return self.ADDRESS_MAP.keys()

    def echo(self, address):
        if not address in self.ADDRESS_MAP:
            return None
        address = self.ADDRESS_MAP[address]
        packetOut = self.makePacket(address, self.CMD_ECHO)
        with self.lock:
            self.send(packetOut)
            packetIn = self.receive(len(packetOut), nRetries = 1)
        packetIn = self.parsePacket(packetIn)
        if not packetIn:
            return False
        
        #logging.debug("Parsed packet: %s" % str(packetIn))
        (address2, cmd2, params2, request2) = packetIn
        
        if (address2 != address) or (cmd2 != self.CMD_ECHO) or request2:
            return False
        return True

    def getParameter(self, address, command, params = None, nRetries = 3):
        if not address in self.ADDRESS_MAP:
            return None
        address = self.ADDRESS_MAP[address]
        params2 = None
        with self.lock:
            packetOut = self.makePacket(address, command, params)
            for nTry in range(nRetries):
                self.send(packetOut)
                packetIn = self.receivePacket()
                packetIn = self.parsePacket(packetIn)
                if not packetIn:
                    time.sleep(0.1)
                    continue
                (address2, cmd2, params2, request2) = packetIn
                if (address2 != address) or (cmd2 != command) or request2:
                    time.sleep(0.1)
                    continue
                break
        return params2

    def reboot(self, address):
        if not address in self.ADDRESS_MAP:
            return None
        with self.lock:
            data = self.makePacket(self.ADDRESS_MAP[address], self.CMD_REBOOT)
            self.send(data)
            recv = self.receive(8)
        return
        
    def send(self, data):
        #self.ser.rts = True
        self.ser.write(data)
        self.ser.flush()
        logging.debug("Sent %s" % prettyFormat(data))
        #self.ser.rts = False
        return

    def receive(self, size, nRetries = 3):
        for i in range(nRetries):
            recv = bytearray(self.ser.read(size))
            logging.debug("Recv %s" % prettyFormat(recv))
            if not recv:
                continue
            break
        if len(recv) == size:
            return recv
        return None

    def receivePacket(self, nRetries = 3):
        result = ""
        while True:
            recv = self.ser.read()
            if not recv:
                result = bytearray(result)
                logging.debug("Recv %s" % prettyFormat(result))
                return result
            result += recv
                    
    def wait(self, data, nRetries = 3):
        for i in range(nRetries):
            recv = self.ser.read(len(data))
            if len(recv) == 0:
                continue
            if recv == data:
                #logging.debug("Received expected bytes (%s)" % (recv.encode('hex')))
                return True
            else:
                logging.warning("Received unexpected bytes (%s), expecting (%s)" % (recv.encode('hex'), data.encode('hex')))
                return False
        logging.warning("Receive timeout")
        return False

def main(args):
  if args.debug:
    level = logging.DEBUG
  else:
    level = logging.INFO
  logging.basicConfig(level = level)
  logging.debug(args)

  ser = rs485.RS485(args.port, args.baudrate, timeout = 0.5, writeTimeout = 0.24)
  crc = CRC(8, CRC_POLYNOMIAL, CRC_INITIAL)
  bus = Bus(ser, crc)

  if args.echo:
      logging.info("Echo...")
      for i in range(args.repeat):
          if args.node: addrlist = [args.node]
          else: addrlist = bus.getAddresses()
          for addr in addrlist:
            success = bus.echo(addr)
            if success == None:
                logging.error("Unknown Node")
                sys.exit(1)
            if success:
                logging.info("%s: SUCCESS" % addr)
            else:
                logging.warning("%s: FAILED" % addr)
            time.sleep(0.1)

  if args.get:
      logging.info("Getting parameter %d" % args.get)
      for i in range(args.repeat):
          params = bus.getParameter(args.node, args.get)
          if params == None:
              logging.error("No response")
          else:
              logging.info("Got %s" % prettyFormat(params, "hex"))
          time.sleep(0.1)

  if args.set:
      paramsIn = [int(x) for x in args.values.split(',')]
      logging.info("Setting parameter %d" % args.set)
      for i in range(args.repeat):
          params = bus.getParameter(args.node, args.set, bytearray(paramsIn))
          if params == None:
              logging.error("No response")
          else:
              logging.info("Got %s" % prettyFormat(params, "hex"))
          time.sleep(0.1)
          
  if args.reboot:
      logging.info("Rebooting...")
      bus.reboot(args.node)

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

if __name__ == "__main__":
  parser = argparse.ArgumentParser(description = 'TinySafeBoot command-line tool')

  parser.add_argument('-p', '--port', help = 'Serial port device')
  parser.add_argument('-b', '--baudrate', help = 'Serial baudrate (default 19200)', type = int, default = 19200)
  parser.add_argument('-n', '--node', help = 'Node identifier')
  parser.add_argument('-R', '--reboot', help = 'Reboot', action = 'store_true', default = False)
  parser.add_argument('-e', '--echo', help = 'Ping node for echo', action = 'store_true', default = False)
  parser.add_argument('-r', '--repeat', help = 'Repeat N times', type = int, default = 1)
  parser.add_argument('-d', '--debug', help = 'Debug', action = 'store_true', default = False)
  parser.add_argument('-g', '--get', help = 'Get parameter', type = int)
  parser.add_argument('-s', '--set', help = 'Set parameter', type = int)
  parser.add_argument('-v', '--values', help = 'Values')

  args = parser.parse_args(sys.argv[1:])
  config = readConfig(vars(args))
  
  main(args)
