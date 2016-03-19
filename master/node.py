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
    
    def __init__(self, bus, address):
        self.bus = bus
        self.address = address
        
    def checkEcho(self):
        return self.bus.echo(self.address)
       
    def getFinished(self):
        params = self.bus.getParameter(self.address, self.CMD_DONE)
        if params: return self.decodeBool(params)
        return None
    
    def setFinished(self, isFinished):
        params = self.bus.getParameter(self.address, self.CMD_DONE, self.encodeBool(isFinished))
        if params: return self.decodeBool(params)
        return None

    def encodeBool(self, value):
        if value: byte = 1
        else: byte = 0
        return bytearray([byte])

    def decodeBool(self, params):
        return (params[0] != 0)
 
    def encodeWord(self, value):
        low = value & 0xFF
        high = (value >> 8) & 0xFF
        return bytearray([low, high])

    def decodeWord(self, params):
        return (params[0] + params[1] * 0xFF)

    def encodeBits(self, valueList):
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


class Bomb(NodeBase):
    CMD_TIME        = 16
    CMD_LEDS        = 17
    
    def __init__(self, bus):
        NodeBase.__init__(self, bus, "BOMB")

    def getTime(self):
        params = self.bus.getParameter(self.address, self.CMD_TIME)
        if params: return (params[0], params[1])
        return None
    
    def setTime(self, minutes, seconds):
        params = self.bus.getParameter(self.address, self.CMD_TIME, bytearray([minutes, seconds]))
        if params: return (params[0], params[1])
        return None
        
    def getLeds(self):
        params = self.bus.getParameter(self.address, self.CMD_LEDS)
        if params: return self.decodeBits(params, 10)
        return None
        
    def setLeds(self, ledMap):
        params = self.bus.getParameter(self.address, self.CMD_LEDS, self.encodeBits(ledMap))
        if params: return self.decodeBits(params, 10)
        return None
        
class Valve(NodeBase):
    CMD_DIGIT       = 16

    def __init__(self, bus):
        NodeBase.__init__(self, bus, "VALVE")
    
    def getDigit(self):
        return self.getByte(self.CMD_DIGIT)
    
    def setDigit(self, digit):
        return self.setByte(self.CMD_DIGIT, digit)

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
