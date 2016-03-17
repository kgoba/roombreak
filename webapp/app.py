from flask import Flask, render_template

app = Flask(__name__)

@app.route('/')
def index():
    rowlist = (("WC", "Alive"), ("AURORA", "Down"))
    return render_template('index.html', rowlist = rowlist)

@app.route('/cakes')
def cakes():
    return 'Yummy cakes!'

if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0')

