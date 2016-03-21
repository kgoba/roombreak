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
    master = app.config['MASTER']
    if not master:
        app.logger.warning("Master object not found")
        return ""
    try:
        minutes = master.minutes
        seconds = master.seconds
        timeLeft = "%02d:%02d" % (minutes, seconds)
    except:
        app.logger.warning("Error while getting time update")
        timeLeft = ""
        minutes = ''
        seconds = ''
        
    return jsonify(timeLeft = timeLeft, minutes = minutes, seconds = seconds)

# return the node statuses
@app.route('/_status')
def get_status():
    #a = request.args.get('a', 0, type=int)
    #b = request.args.get('b', 0, type=int)
    master = app.config['MASTER']
    if not master:
        app.logger.warning("Master object not found")
        return ""
    
    try:
        response = master.getStatus()
    except:
        response = {}
        app.logger.warning("Error while getting status update")
    
    return jsonify(response)

# finish/reset node
@app.route('/_setDone')
def reset():
    master = app.config['MASTER']
    if not master:
        app.logger.warning("Master object not found")
        return ''

    try:
        name = request.args.get('id', '', type=str)
        isDone = request.args.get('done', None, type=str)
        if isDone == 'true':
            master.setDone(name, True)
        elif isDone == 'false':
            master.setDone(name, False)
    except Exception as e:
        app.logger.warning("Error while resetting node (%s)" % str(e))
    return ''
    
def startServer():        
    app.run(debug=False, host='0.0.0.0', port = 8088)

# start the server with the 'run()' method
if __name__ == '__main__':
    startServer()
