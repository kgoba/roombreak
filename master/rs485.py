#!/usr/bin/env python

PIN_RXD = 17
PIN_TXE = 22
PIN_RX = 15

try:
  #from gpiozero import DigitalOutputDevice
  #pinRXD = 
  #x = DigitalOutputDevice(PIN_RXD)
  #pinTXE = 
  #y = DigitalOutputDevice(PIN_TXE)
  #pinRX = 
  #z = DigitalInputDevice(PIN_RX, pull_up=True)
  pinRXD = None
  pinTXE = None
except:
  pinRXD = None
  pinTXE = None

import time
import serial
import logging

class RS485:
  def __init__(self, device, baudrate = 19200, timeout = 1, writeTimeout = 0.2):
    self.ser = serial.Serial(device, baudrate = baudrate, timeout = timeout, writeTimeout = writeTimeout, 
                    parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE, bytesize=serial.EIGHTBITS)
    return

  def write(self, data):
    if pinRXD != None: pinRXD.on()
    if pinTXE != None: pinTXE.on()
    result = self.ser.write(data)
    self.ser.flushOutput()
    if pinTXE != None: 
      pinTXE.off()
      pinRXD.off()
      #echo = self.ser.read(len(data))
    return result

  def read(self, size = 1):
    logging.debug("Reading %d bytes" % size)
    return self.ser.read(size = size)

  def flush(self):
    #self.ser.flushOutput()
    #if pinRXD != None: pinRXD.off()
    return
