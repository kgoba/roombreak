# import the Flask class from the flask module
from flask import Flask, render_template
from flask import jsonify, request

import os
import sys
import logging

#sys.path.append(sys.path[0] + '/../master')
sys.path.append('../master')
from rs485 import RS485
from bus import Bus
import node

# create the application object
app = Flask(__name__)
nodes = {}

# use decorators to link the function to a url
@app.route('/')
def home():
    return render_template('index.html')

# return time left
@app.route('/_time')
def get_time():
    try:
        (minutes, seconds)= nodes['BOMB'].getTime()
        timeLeft = "%02d:%02d" % (minutes, seconds)
    except:
        timeLeft = ""
    return jsonify(timeLeft = timeLeft)

# return the node statuses
@app.route('/_status')
def get_status():
    #a = request.args.get('a', 0, type=int)
    #b = request.args.get('b', 0, type=int)
    response = {}
    
    response['status'] = "Pauze"
    response['doorsOpen'] = False
    
    response['BOMB'] = {}
    try:
        leds = nodes['BOMB'].getLeds()
        response['BOMB']['timeLeft'] = leds
        response['BOMB']['alive'] = True
    except:
        timeLeft = None
        response['BOMB']['alive'] = False

    response['VALVE'] = {}
    try:
        valveDigit = str(nodes['VALVE'].getDigit())
        response['VALVE']['digit'] = valveDigit
        response['VALVE']['alive'] = True
    except:
        valveDigit = None
        response['VALVE']['alive'] = False

    #return jsonify(status="Pauze", timeLeft=timeLeft, valveDigit=valveDigit, doorsOpen=False)
    return jsonify(response)

def readConfig():
    home = os.path.expanduser("~")    
    values = dict()
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

# start the server with the 'run()' method
if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    try:
        config = readConfig()
        ser = RS485(config['serial'], 19200, timeout = 0.25, writeTimeout = 0.5)
        bus = Bus(ser)
        nodes['BOMB'] = node.Bomb(bus)
        nodes['VALVE'] = node.Valve(bus)
    except:
        logging.warning("Could not initialize bus communication")
    
    app.run(debug=False, host='0.0.0.0', port = 8088)
