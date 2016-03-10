#!/usr/bin/env python
from gpiozero import DigitalOutputDevice
from gpiozero import DigitalInputDevice
from signal import pause

import time
import rs485

PIN_NODE_RST = 27
PIN_START_BTN = 23
PIN_EXIT_EN = 24

PIN_SNAKE_EN = 2
PIN_EASTER_EN = 3
PIN_SNAKE_DONE = 25


rstPin = DigitalOutputDevice(PIN_NODE_RST)

btnPin = DigitalInputDevice(PIN_START_BTN)
outPin = DigitalOutputDevice(PIN_EXIT_EN)

snakeOnPin = DigitalOutputDevice(PIN_SNAKE_EN)

#outPin.source = btnPin.values

#pause()

#rstPin.on()
#time.sleep(0.1)
#rstPin.off()
#time.sleep(1)

snakeOnPin.off()
time.sleep(10)
snakeOnPin.on()

#ser = rs485.RS485('/dev/ttyAMA0')

#ser.write('Test')
#print ser.read(4)

