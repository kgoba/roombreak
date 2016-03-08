#!/usr/bin/env python

import argparse
import logging
import serial
import struct
import time
import sys

CRC_POLYNOMIAL  = 0x21
CRC_INITIAL     = 0xFF

logging.basicConfig(level = logging.INFO)

parser = argparse.ArgumentParser(description = 'TinySafeBoot command-line tool')

parser.add_argument('-d', help = 'Serial port device', metavar='DEV', dest = 'DEV')
parser.add_argument('-b', '--baudrate', help = 'Serial baudrate (default 19200)', type = int, default = 19200)
parser.add_argument('-n', '--node', help = 'Node identifier')
parser.add_argument('-R', '--reboot', help = 'Reboot', action = 'store_true', default = False)
parser.add_argument('-e', '--echo', help = 'Ping node for echo', action = 'store_true', default = False)
parser.add_argument('-r', '--repeat', help = 'Repeat N times', type = int, default = 1)

args = parser.parse_args(sys.argv[1:])

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
        'PLAYER': 26
    }
    
    def __init__(self, serial, crc):
        self.ser = serial
        self.crc = crc
        
    def prettyFormat(self, data):
        str = " ".join('%02X' % b for b in data)
        return '[' + str + '] (%d b)' % len(data)
        
    def packet(self, address, cmd, params = bytearray()):
        data = bytearray(self.PREFIX + [address, cmd, len(params)])
        if params: data += params
        data.append(self.crc.compute(data))
        return data
       
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

    def echo(self, address):
        if not address in self.ADDRESS_MAP:
            return None
        data = self.packet(self.ADDRESS_MAP[address], self.CMD_ECHO)
        self.send(data)
        recv = self.receive(len(data), nRetries = 1)
        success = self.check(data, recv)
        return success

    def reboot(self, address):
        data = self.packet(address, self.CMD_REBOOT)
        self.send(data)
        recv = self.receive(8)
        return
        
    def send(self, data):
        #self.ser.rts = True
        self.ser.write(data)
        self.ser.flush()
        logging.debug("Sent %s" % self.prettyFormat(data))
        #self.ser.rts = False
        return

    def receive(self, size, nRetries = 3):
        for i in range(nRetries):
            recv = bytearray(self.ser.read(size))
            logging.debug("Recv %s" % self.prettyFormat(recv))
            if not recv:
                continue
            if len(recv) == size:
                return recv
            break
        return None
        
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

logging.debug(args)

ser = serial.Serial(args.DEV, args.baudrate, timeout = 0.2, write_timeout = 0.2, rtscts = False)
crc = CRC(8, CRC_POLYNOMIAL, CRC_INITIAL)
bus = Bus(ser, crc)

if args.echo:
    logging.info("Echo...")
    for i in range(args.repeat):
        success = bus.echo(args.node)
        if success == None:
            logging.error("Unknown Node")
            sys.exit(1)
        if success:
            logging.info("SUCCESS")
        else:
            logging.warning("FAILED")
        time.sleep(0.2)

if args.reboot:
    logging.info("Rebooting...")
    bus.reboot(args.node)
