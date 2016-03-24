#!/usr/bin/env python

ADDRESS_PBX     = 17
ADDRESS_BOMB    = 18
ADDRESS_P2K     = 19
ADDRESS_RFID    = 20
ADDRESS_KEY     = 21
ADDRESS_FLOOR   = 22
ADDRESS_MAP     = 23
ADDRESS_VALVE   = 24
ADDRESS_WC      = 25
ADDRESS_PLAYER  = 26
ADDRESS_DIMMER  = 27

class NodeBase:
    CMD_DONE = 2
    CMD_STATUS = 3
    
    MAX_FAILURES = 3
    
    def __init__(self, bus, address):
        self.bus = bus
        self.address = address
        self.done = None
        self.unsuccessful = 0
        
    def checkEcho(self):
        return self.bus.echo(self.address)

    def update(self):
        self.getDone()

    def encodeBool(self, value):
        if value == None: return None
        if value: byte = 1
        else: byte = 0
        return bytearray([byte])

    def decodeBool(self, params):
        return (params[0] != 0)

    def encodeByte(self, value):
        if value == None: return None
        return bytearray([value])

    def decodeByte(self, params):
        return (params[0])
         
    def encodeWord(self, value):
        if value == None: return None
        low = value & 0xFF
        high = (value >> 8) & 0xFF
        return bytearray([low, high])

    def decodeWord(self, params):
        return (params[0] + params[1] * 0xFF)

    def encodeBits(self, valueList):
        if valueList == None: return None
        result = []
        mask = 1
        byte = 0
        for value in valueList:
            if value: byte |= mask
            if mask == 128:
                result.append(byte)
                byte = 0
                mask = 1
            else:
                mask <<= 1
        if mask != 1:
            result.append(byte)
        return bytearray(result)

    def decodeBits(self, params, maxBits = None):
        result = []
        mask = 1
        nBits = 0
        for byte in params:
            mask = 1
            for nBit in range(8):
                result.append( (byte & mask) != 0)
                mask <<= 1
                nBits += 1
                if maxBits and nBits >= maxBits:
                    return tuple(result)
        return tuple(result)
        
    def getByte(self, cmd):
        params = self.bus.getParameter(self.address, cmd)
        if params: return (params[0])
        return None
    
    def setByte(self, cmd, byte):
        params = self.bus.getParameter(self.address, cmd, bytearray([byte]))
        if params: return (params[0])
        return None

    def tryCommand(self, cmd, params = None):
        params = self.bus.getParameter(self.address, cmd, params)
        if params:
            self.unsuccessful = 0
        else:
            self.unsuccessful += 1
        return params

    def getDone(self, newValue = None):
        params = self.tryCommand(self.CMD_DONE, self.encodeBool(newValue))
        if params:
            self.done = self.decodeBool(params)
            return self.done
        return None

    def queryStatus(self):
        params = self.bus.getParameter(self.address, self.CMD_STATUS)
        if params == None:
            self.unsuccessful += 1
        else:
            self.unsuccessful = 0
        return params
        
    def isAlive(self):
        return self.unsuccessful < self.MAX_FAILURES
    
    def getAllValues(self):    
        values = {}
        values['done'] = self.done
        values['alive'] = self.isAlive()
        return values

class Bomb(NodeBase):
    CMD_TIME        = 16
    CMD_LEDS        = 17
    CMD_ENABLE      = 18
    
    def __init__(self, bus):
        NodeBase.__init__(self, bus, "BOMB")
        self.minutes = None
        self.seconds = None
        self.leds = None
        self.enabled = None

    def getAllValues(self):
        values = NodeBase.getAllValues(self)
        values['leds'] = self.leds
        values['enabled'] = self.enabled
        return values

    def update(self):
        self.getDone()
        self.getLeds()
        self.getEnabled()
        #self.getTime()

    def getTime(self):
        params = self.tryCommand(self.CMD_TIME)
        if params: 
            self.minutes = params[0]
            self.seconds = params[1]
            return (self.minutes, self.seconds)
        return None
    
    def getLeds(self):
        #return self.leds
        params = self.bus.getParameter(self.address, self.CMD_LEDS)
        if params: 
            self.leds = self.decodeBits(params, 10)
            return self.leds
        return None

    def getEnabled(self):
        #return self.enabled
        params = self.bus.getParameter(self.address, self.CMD_ENABLE)
        if params: 
            self.enabled = self.decodeBool(params)
            return self.enabled
        return None
        
    def setTime(self, minutes, seconds):
        params = self.bus.getParameter(self.address, self.CMD_TIME, bytearray([minutes, seconds]))
        if params: return (params[0], params[1])
        return None
                
    def setLeds(self, ledMap):
        params = self.bus.getParameter(self.address, self.CMD_LEDS, self.encodeBits(ledMap))
        if params: return self.decodeBits(params, 10)
        return None

    def setEnabled(self, enabled):
        params = self.bus.getParameter(self.address, self.CMD_ENABLE, self.encodeBool(enabled))
        if params: return self.decodeBool(params)
        return None

class Valve(NodeBase):
    CMD_DIGIT       = 16

    def __init__(self, bus):
        NodeBase.__init__(self, bus, "VALVE")
        self.digit = None

    def getAllValues(self):
        values = NodeBase.getAllValues(self)
        values['digit'] = self.digit
        return values

    def update(self):
        self.getDone()
        self.getDigit()
            
    def getDigit(self, newValue = None):
        params = self.tryCommand(self.CMD_DIGIT, self.encodeByte(newValue))
        if params:
            self.digit = self.decodeByte(params)
            return self.digit
        return None
    
class Floor(NodeBase):
    CMD_SENSORDONE       = 18

    def __init__(self, bus):
        NodeBase.__init__(self, bus, "FLOOR")
        self.sensorDone = None

    def getAllValues(self):
        values = NodeBase.getAllValues(self)
        values['sensorDone'] = self.sensorDone
        return values

    def update(self):
        self.getDone()
        self.getSensorDone()
            
    def getSensorDone(self, newValue = None):
        params = self.tryCommand(self.CMD_SENSORDONE, self.encodeBits(newValue))
        if params:
            self.sensorDone = self.decodeBits(params, 10)
            return self.sensorDone
        return None

class RFID(NodeBase):

    def __init__(self, bus):
        NodeBase.__init__(self, bus, "RFID")
        self.sensorDone = None

    def update(self):
        self.getDone()

class Key(NodeBase):

    def __init__(self, bus):
        NodeBase.__init__(self, bus, "KEY")
        self.sensorDone = None

    def update(self):
        self.getDone()

class Map(NodeBase):

    def __init__(self, bus):
        NodeBase.__init__(self, bus, "MAP")
        self.sensorDone = None

    def update(self):
        self.getDone()
  
class PBX(NodeBase):

    def __init__(self, bus):
        NodeBase.__init__(self, bus, "PBX")
        self.sensorDone = None

    def update(self):
        self.getDone()

class P2K(NodeBase):

    def __init__(self, bus):
        NodeBase.__init__(self, bus, "P2K")
        self.sensorDone = None

    def update(self):
        self.getDone()
        
class WC(NodeBase):

    def __init__(self, bus):
        NodeBase.__init__(self, bus, "WC")
        self.sensorDone = None

    def update(self):
        self.getDone()

class Dimmer(NodeBase):
    CMD_DIMMER1 = 16
    CMD_DIMMER2 = 17

    def __init__(self, bus):
        NodeBase.__init__(self, bus, "DIMMER")
        self.dimmer1 = None

    def setDimmer1(self, newValue):
        params = self.tryCommand(self.CMD_DIMMER1, self.encodeByte(newValue))
        if params:
            self.dimmer1 = self.decodeByte(params)
            return self.dimmer1
        return None

    def setDimmer2(self, newValue):
        params = self.tryCommand(self.CMD_DIMMER2, self.encodeByte(newValue))
        if params:
            self.dimmer2 = self.decodeByte(params)
            return self.dimmer2
        return None
        
class Player(NodeBase):
    CMD_TRACK1       = 16
    CMD_TRACK2       = 17
    CMD_TRACK3       = 18
    
    def __init__(self, bus):
        NodeBase.__init__(self, bus, "PLAYER")

    def getTrack1(self):
        return self.getByte(self.CMD_TRACK1)
    
    def setTrack1(self, track):
        return self.setByte(self.CMD_TRACK1, track)

    def getTrack2(self):
        return self.getByte(self.CMD_TRACK2)
    
    def setTrack2(self, track):
        return self.setByte(self.CMD_TRACK2, track)

    def getTrack3(self):
        return self.getByte(self.CMD_TRACK3)
    
    def setTrack3(self, track):
        return self.setByte(self.CMD_TRACK3, track)
