from bottle import route, static_file, debug, run, get, redirect
from bottle import post, request
import os, inspect, json
import golduino

#enable bottle debug
debug(True)

golduino.init('/dev/ttyACM0', 38400)

# WebApp route path
routePath = '/golduino'
# get directory of WebApp (bottlejquery.py's dir)
rootPath = os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe())))

@route(routePath)
def rootHome():
    return redirect(routePath+'/index.html')

@route(routePath + '/<filename:re:.*\.html>')
def html_file(filename):
    return static_file(filename, root=rootPath)

@route(routePath + '/reset')
def reset():
    golduino.reset()
    return '<h3>Reset!</h3>'

@route(routePath + '/step')
def step():
    golduino.step()
    return '<h3>Step!</h3>'

@route(routePath + '/continue')
def continue_running():
    golduino.run()
    return '<h3>Continue!</h3>'

@get('/_interval')
def _interval():
    interval = int(request.query['interval'])
    golduino.configure()
    golduino.configure_iteration_interval(interval)
    golduino.run()

@get('/_seed')
def _seed():
    seed = request.query['seed']
    golduino.configure()
    golduino.configure_seed(seed)
    golduino.step()

@get('/_color')
def _color():
    rgb = None
    choice = request.query['choice']
    if choice == 'Input_Color':
        red = int(request.query['red'])
        green = int(request.query['green'])
        blue = int(request.query['blue'])
        rgb = (red,green,blue)
    golduino.configure()
    golduino.configure_color(choice, rgb)
    golduino.run()

run(host='0.0.0.0', port=8080, reloader=True)
