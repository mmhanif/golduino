import serial

SeedChoices = ['Input_Seed', 'R_Pentamino', 'Acorn', 'Diehard', 'Pattern_10', 'Pattern_5x5', 'Random_Seed', 'Random_Choice']
SeedChoiceDict = dict(zip(SeedChoices, range(len(SeedChoices))))

ColorChoices = ['Input_Color', 'Red', 'Green', 'Blue', 'Random_Color', 'Continuous_Random']
ColorChoiceDict = dict(zip(ColorChoices, range(len(ColorChoices))))


_ser = serial.Serial()
_mode = ''

def init(port, baudrate):
    _ser.port = port
    _ser.baudrate = baudrate
    _ser.open()

def configure():
    global _mode
    if _mode != 'Configure':
        _ser.write('c')
        _mode = 'Configure'

def step():
    global _mode
    if _mode != 'Running_Step':
        _ser.write('s')
        _mode = 'Running_Step'
    else:
        _ser.write('n')

def run():
    global _mode
    if _mode != 'Running_Continuous':
        _ser.write('t')
        _mode = 'Running_Continuous'

def reset():
    _ser.write('r')
    # _mode stays whatever it was before

# If seed_choice == 'Input_Seed',
# points should contain a list of two-integer tuples
def configure_seed(seed_choice, points=[]):
    if _mode == 'Configure':
        if seed_choice in SeedChoices:
            _ser.write('e%d' % SeedChoiceDict[seed_choice])
            if seed_choice == 'Input_Seed':
                for pt in points:
                    _ser.write(',%d,%d' % pt)
                _ser.write(',-1')

# If color_choice == 'Input_Color',
# rgb should be a three-integer tuple, with each integer in the range 0-255
def configure_color(color_choice, rgb=None):
    if _mode == 'Configure':
        if color_choice in ColorChoices:
            _ser.write('l%d' % ColorChoiceDict[color_choice])
            if color_choice == 'Input_Color':
                _ser.write(',%d,%d,%d' % rgb)

def configure_iteration_interval(interval):
    if _mode == 'Configure':
        _ser.write('i%d' % interval)
