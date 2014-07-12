#!/usr/bin/python
# uv1-explore.py
# Automated exploration main procedure for RPI-UV1 drone.
#
# Author : Oren Camber
# Date   : 2014-06-21

# Import required Python libraries
import subprocess
import time
import RPi.GPIO as GPIO

# Use BCM GPIO references instead of physical pin numbers
GPIO.setmode(GPIO.BCM)

# Set up for direct control of lights and laser
GPIO.setup(LIGHTS_GPIO, GPIO.OUT)
GPIO.setup(LASER_GPIO, GPIO.OUT)

IMG_FILE = '/home/pi/UV1-IMG-%Y%m%d%H%M%S-'
LOG_FILE = '/home/pi/UV1-LOG.txt'
SENSOR_FILE = '/dev/shm/sensor_data'
SENSORD_CMD = '/home/pi/src/uv1/sensord'
MOTORS_CMD = '/home/pi/src/uv1/motors '
LIGHTS_CMD = '/home/pi/src/uv1/lights '
LASER_CMD = '/home/pi/src/uv1/laser '
RESET_SENSORS_CMD = '/home/pi/src/uv1/reset_sensors '
PHOTO_CMD = 'raspistill -n -o '
VIDEO_CMD = 'raspivid -n '
LIGHTS_GPIO = 14
LASER_GPIO  = 7
MIN_AVG_LIGHT_LEVEL = 100
PARTIAL_TURN = 'FR240'

sensor_daemon_proc = None
motors_proc = None
camera_proc = None
other_proc = None
sensor_signals = None
survey_data = None
movement_result = ['degrees': 0, 'cm': 0]
log_file = None

# Turn off lights and laser
laser_on = False
lights_on = True

interrupt_signal_received = False

def main(args):    
    
    # Initialize log file
    log_file = open(LOG_FILE, 'w')
    log_file.write('[\n')

    # Turn on sensors
    sensor_daemon_proc = subprocess.Popen([SENSORD_CMD])
    
    while not interrupt_signal_received:

        # Make sure that lights and laser are set correctly
        GPIO.output(LIGHTS_GPIO, lights_on)
        GPIO.output(LASER_GPIO, laser_on)

        # Survey environment and log results
        survey_data = survey_surroundings()
        write_log_entry(movement_result, survey_data)

        # Turn on lights if necessary
        lights_on = ambient_light(survey_data) < MIN_AVG_LIGHT_LEVEL
        GPIO.output(LIGHTS_GPIO, lights_on)
        
        # Move to new position
        new_vector = determine_best_vector(survey_data)
        movement_result = proceed(new_vector)
        
        # React to sensor input
        sensor_signals = read_sensors()
        if sensor_signals.sound:
            interrupt_signal_received = True
            break
        if sensor_signals.obstacle or sensor_signals.touch:
            reverse_away_from_obstacle()            
            continue
    
    survey_data = [['sensors':sensor_signals, 'image':'']]
    write_log_entry(movement_result, survey_data)
    
    log_file.write("{}]\n")
    log_file.close()
    sensor_daemon_proc.kill()

def sensor_json(sensors):
    return "{touch:" + str(sensors.touch) \
        + ", obstacle:" + str(sensors.obstacle)
        + ", sound:" + str(sensors.sound)
        + ", range:" + str(sensors.range) + "}"

def movement_json(movement):
    return "{degrees:" + str(movement.degrees) + ", cm:" + str(movement.cm) + "}"

def survey_json(survey):
    result = "["
    item_count = 0
    for item in survey:
        if item_count > 0:
            result = result + ',\n'
        else:
            result = result + '\n'
        item_count = item_count + 1
        result = result + "{sensors:" + sensor_json(item.sensors) + ", image:'" + item.image + "'}"    
    return result + "\n]"
    
def write_log_entry(movement, survey):
    log_file.write("{movement:" + movement_json(movement) + ", survey:" + survey_json(survey) + "}, \n" )
    
def read_sensors():
    results = ['touch':0, 'obstacle':0, 'sound':0, 'range':0 ]
    with open(SENSOR_FILE, 'r') as file:
        file_text = file.read()
    if file_text[0:1]=='T' and file_text[1:2]=='+':
        results.touch = 1
    if file_text[2:3]=='O' and file_text[3:4]=='+':
        results.obstacle = 1
    if file_text[4:5]=='S' and file_text[5:6]=='+':
        results.sound = 1
    if file_text[6:7]=='R':
        results.range = int(file_text[7:10], base=10)
    return results
    
def ambient_light:
    # TEMP CODE! Replace with photo light level analysis!
    return MIN_AVG_LIGHT_LEVEL + 50

def survey_surroundings():
    results = []
    img_file_prefix = datetime.datetime.now().strftime(IMG_FILE)
    for i in range(0, 9):
        img_file = img_file_prefix + str(i) + '.jpg'
        subprocess.call([MOTORS_CMD + PARTIAL_TURN])
        subprocess.call([PHOTO_CMD + img_file])
        sensor_signals = read_sensors()
        if sensor_signals.sound:
            interrupt_signal_received = True
            break
        results.append(['sensors':sensor_signals, 'image':img_file])
    return results
    
def log_survey(survey_data):
    for item in survey_data:
        # Write sensor values and photo filename into log file
        pass

def log_movement(vector, survey):
    # Write vector.direction and vector.distance
    log_file.write("{deg:" + str(vector.degrees) + ", cm:" + str(vector.cm) + ", survey:[")
    
                + "0, touch:" + str(sensor_signals.touch)
                + ", obstacle:" + str(sensor_signals.obstacle)
                + ", sound:" + str(sensor_signals.sound)
                + ", range:" + str(sensor_signals.range) + "} ]\n")
    