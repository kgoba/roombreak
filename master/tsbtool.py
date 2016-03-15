#!/usr/bin/env python

import argparse
import logging
import serial
import struct
import rs485
import time
import sys

from bus import CRC
from bus import Bus

logging.basicConfig(level = logging.DEBUG)

parser = argparse.ArgumentParser(description = 'TinySafeBoot command-line tool')

parser.add_argument('-d', help = 'Serial port device', metavar='DEV', dest = 'DEV')
parser.add_argument('-b', '--baudrate', help = 'Serial baudrate (default 19200)', type = int, default = 19200)
parser.add_argument('-R', '--run', help = 'Run application', action = 'store_true', default = False)
parser.add_argument('-c', '--connect', help = 'Connect (with optional password)', nargs = '?', const = '', metavar = 'PASSWORD')
parser.add_argument('-r', '--read', help = 'Read into file (HEX)', type = argparse.FileType('w'))
parser.add_argument('-w', '--write', help = 'Write from file (HEX)', type = argparse.FileType('r'))
parser.add_argument('-t', '--type', help = 'Access type', choices = ['flash', 'EEPROM'], default = 'flash')
parser.add_argument('-p', '--password', help = 'Change password', nargs = '?', const = '', metavar = 'NEWPASSWORD')

args = parser.parse_args(sys.argv[1:])

class TSB:
    N_RETRIES = 5
    INFO_SIZE = 2*8
    CONFIRM   = '!'
    REQUEST   = '?'
    
    CMD_RD_FLASH    = 'f'
    CMD_WR_FLASH    = 'F'
    CMD_RD_EEPROM   = 'e'
    CMD_WR_EEPROM   = 'E'
    CMD_RD_UDATA    = 'c'
    CMD_WR_UDATA    = 'C'
    CMD_RUN         = 'r'
    
    def __init__(self, serial):
        self.ser = serial
        self.connected = False
        
    def connect(self, password):
        for i in range(self.N_RETRIES):
            self.send('@@@%s' % password)
            info = self.readInfo()
            if info:
                if info[0] == 'TSB' or info[0] == '\xd5SB':
                    self.pageSize = 2 * info[6]
                    self.flashSize = 2 * info[7]
                    self.eepromSize = 2 * info[8]
                    logging.info("Page size %d, flash size %d, EEPROM size %d" % (self.pageSize, self.flashSize, self.eepromSize))
                    self.connected = True
            
                    if self.wait(self.CONFIRM):
                        return True

            logging.warn("Timeout; retrying...")
            time.sleep(1)
        
        logging.error("Bootloader signature not found")
        return False
        
    def readInfo(self):
        info = self.receive(self.INFO_SIZE)        
        if info:
            logging.debug("Got %d bytes : [%s]" % (len(info), info.encode('hex')))
            info = struct.unpack('<3sHBBBBBHHH', info)
        return info

    def send(self, data):
        #self.ser.rts = True
        self.ser.write(data)
        self.ser.flush()
        #self.ser.rts = False
        return

    def receive(self, size, nRetries = 3):
        for i in range(nRetries):
            recv = self.ser.read(size)
            #logging.debug("Received %d bytes" % len(recv))
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
        
    def writeFlash(self, data):
        logging.debug("Sending write flash command...")
        self.send(self.CMD_WR_FLASH)
        
        nPages = len(data) / self.pageSize
        logging.info("%d pages to send" % nPages)

        iPage = 0
        iData = 0
        while iData < len(data):
            if not self.wait(self.REQUEST):
                logging.error("Data request not received, aborting")
                return
            logging.debug("Sending page %d" % iPage)
            #time.sleep(0.05)
            self.send(self.CONFIRM)
            if iData + self.pageSize <= len(data):
                pageData = data[iData : iData + self.pageSize]
            else:
                pageData = data[iData:]
                pageData += bytes(b'\xFF' * (self.pageSize - len(pageData)))
                
            #logging.debug("Sending %d bytes: [%s]" % (len(pageData), pageData.encode('hex')))
            #time.sleep(0.05)
            self.send(pageData)
            iData += self.pageSize
            iPage += 1

        if not self.wait(self.REQUEST):
            logging.error("Data request not received, aborting")

        time.sleep(0.05)
        logging.debug("Confirming command")
        self.send(self.REQUEST)
        time.sleep(0.05)
        if not self.wait(self.CONFIRM):
            logging.warning("Confirmation not received")
            return
            
        logging.info("Flash write successful")
        return
        
    def readFlash(self):
        logging.debug("Sending read flash command...")
        self.send(self.CMD_RD_FLASH)
        
        data = bytes()
        nPages = self.flashSize / self.pageSize
        logging.info("%d pages to receive" % nPages)
        
        for iPage in range(nPages):
            self.send(self.CONFIRM)
            logging.debug("Receiving page %d" % iPage)
            pageData = self.receive(self.pageSize)
            if not pageData:
                logging.error("Page data not received")
                return bytes()
            #logging.debug("Page data [%s]" % pageData.encode('hex'))
            data += pageData

        if not self.wait(self.CONFIRM):
            logging.warning("Confirmation not received")
            return None

        logging.info("Flash write successful")
        return data
        
    def startApp(self):
        self.send(self.CMD_RUN)
        return
        
    def setPassword(self, newPassword):
        self.send(self.CMD_RD_UDATA)
        userData = bytearray(self.receive(self.pageSize))
        if not self.wait(self.CONFIRM):
            logging.warning("Confirmation not received")
            return None
        for i in range(0, self.pageSize - 3):
            if i < len(newPassword):
                userData[3 + i] = newPassword[i]
            else:
                userData[3 + i] = 0xFF
        self.send(self.CMD_WR_UDATA)
        if not self.wait(self.REQUEST):
            logging.error("Data request not received, aborting")
            return
        logging.debug("Writing user data")
        self.send(self.CONFIRM)
        self.send(userData)
        userData2 = bytearray(self.receive(self.pageSize))
        if not self.wait(self.CONFIRM):
            logging.warning("Confirmation not received")
            return None        
        if userData != userData2:
            logging.warning("Verification failed")
            logging.debug("Sent %d bytes : [%s]" % (len(userData), str(userData).encode('hex')))
            logging.debug("Got %d bytes  : [%s]" % (len(userData2), str(userData2).encode('hex')))
            return None        
        return

def hexReader(stream):
    for line in stream:
        line = line.rstrip()
        if not line or line[0] != ':':
            continue
        b = line[1:].decode('hex')
        (count, addr, rType, data, checksum) = struct.unpack('>BHB%dsB' % (len(b) - 5), b)
        if len(data) != count:
            raise Exception("Invalid data size")
        yield (rType, addr, data)
    return

def writeHEX(stream, data):
    iData = 0
    while iData < len(data):
        page = data[iData:iData+32]
        stream.write("%s\n" % page.encode('hex'))
        iData += 32
    
    page = data[iData:]
    if page:
        stream.write("%s\n" % page.encode('hex'))
    return

def readHEX(stream):
    fdata = bytes()
    faddress = 0
    for (rType, addr, data) in hexReader(stream):
        if rType == 0:
            if addr != faddress:
                raise Exception("Unexpected address")
            fdata += data
            faddress += len(data)
    logging.debug('Read %d bytes from HEX file' % len(fdata))
    return fdata

#ser = serial.Serial(args.DEV, args.baudrate, timeout = 0.5, write_timeout = 0.5, rtscts = False)
ser = rs485.RS485(args.DEV, args.baudrate, timeout = 0.5, writeTimeout = 0.5)
tsb = TSB(ser)

if args.connect != None:
    bus = Bus(ser)

    logging.info("Rebooting...")
    bus.reboot(args.connect)
    time.sleep(0.25)

    logging.info("Connecting...")
    if not tsb.connect(args.connect):
        logging.error("Connect failed")
        sys.exit(1)
    else:
        logging.info("Connected")

if args.read:
    logging.info("Reading from flash...")
    data = tsb.readFlash()
    writeHEX(args.read, data)
    
if args.write:
    logging.info("Writing to flash...")
    data = readHEX(args.write)    
    tsb.writeFlash(data)

if args.password:
    logging.info("Setting password...")
    tsb.setPassword(args.password)

if args.run:
    logging.info("Running application...")
    tsb.startApp()
