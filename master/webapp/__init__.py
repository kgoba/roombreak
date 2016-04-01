# import the Flask class from the flask module
from flask import Flask, render_template
from flask import jsonify, request

from threading import Thread

# create the application object
app = Flask(__name__)

# use decorators to link the function to a url
@app.route('/')
def home():
    return render_template('index.html')

# return time left
@app.route('/_time')
def get_time():
    try:
        master = app.config['MASTER']
        if not master:
            raise Exception("Master object not found")
        minutes = request.args.get('minutes', '', type=str)
        seconds = request.args.get('seconds', '', type=str)
        if minutes and seconds: 
            master.setTime(int(minutes), int(seconds))

        minutes = master.minutes
        seconds = master.seconds
        timeLeft = "%02d:%02d" % (minutes, seconds)
        return jsonify(success = True, timeLeft = timeLeft, minutes = minutes, seconds = seconds)
    except Exception as e:
        app.logger.warning("Error while getting time update (%s)" % str(e))
        return jsonify(success = False, error = str(e))
        

# return the node statuses
@app.route('/_status')
def get_status():    
    response = {}
    try:
        master = app.config['MASTER']
        if not master:
            raise Exception("Master object not found")
        response = master.getStatus()
        response["success"] = True
        return jsonify(response)
    except Exception as e:
        app.logger.warning("Error while getting status update %s" % str(e))
        return jsonify(success = False, error = str(e))
    return ''
    
# finish/reset node
@app.route('/_setDone')
def reset():
    try:
        master = app.config['MASTER']
        if not master:
            raise Exception("Master object not found")
        name = request.args.get('id', '', type=str)
        isDone = request.args.get('done', None, type=str)
        if isDone == 'true':
            master.setDone(name, True)
        elif isDone == 'false':
            master.setDone(name, False)
        return jsonify(success = True)
    except Exception as e:
        app.logger.warning("Error while resetting node (%s)" % str(e))
        return jsonify(success = False, error = str(e))
    return ''

# finish/reset node
@app.route('/_setGameState')
def setGameState():
    try:
        master = app.config['MASTER']
        if not master:
            raise Exception("Master object not found")
        state = request.args.get('state', '', type=str)
        if state:
            master.setGameState(state)
        return jsonify(success = True)
    except Exception as e:
        app.logger.warning("Error while setting game state (%s)" % str(e))
        return jsonify(success = False, error = str(e))
    return ''
    
def startServer():        
    app.run(debug=False, host='0.0.0.0', port = 8088)

# start the server with the 'run()' method
if __name__ == '__main__':
    startServer()
