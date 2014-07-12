#!/usr/bin/python
# uv1-simple.py
# Automated simple main procedure for RPI-UV1 drone.
#
# Author : Oren Camber
# Date   : 2014-07-11

# Import required Python libraries
import random
import subprocess
import time
import RPi.GPIO as GPIO

# Use BCM GPIO references instead of physical pin numbers
GPIO.setmode(GPIO.BCM)

IMG_FILE = '/home/pi/UV1-IMG-%Y%m%d%H%M%S-'
SENSOR_FILE = '/dev/shm/sensor_data'
SENSORD_CMD = '/home/pi/src/uv1/sensord'
MOTORS_CMD = '/home/pi/src/uv1/motors'
LIGHTS_CMD = '/home/pi/src/uv1/lights'
LASER_CMD = '/home/pi/src/uv1/laser'
RESET_SENSORS_CMD = '/home/pi/src/uv1/reset_sensors'
PHOTO_CMD = 'raspistill -n -o '
VIDEO_CMD = 'raspivid -n '
LIGHTS_GPIO = 14
LASER_GPIO  = 7

GPIO.setwarnings(False)
GPIO.setup(LIGHTS_GPIO, GPIO.OUT)
GPIO.setup(LASER_GPIO, GPIO.OUT)
GPIO.output(LIGHTS_GPIO, False)
GPIO.output(LASER_GPIO, False)

sensor_daemon_proc = None
motors_proc = None
camera_proc = None
other_proc = None
sensor_signals = None

# Turn off lights and laser
laser_on = False
lights_on = True

interrupt_signal_received = False

def main(args): 
    
    # Initialize randomizer
    random.seed()

    # Turn on sensors
    sensor_daemon_proc = subprocess.Popen([SENSORD_CMD])
    
    while not interrupt_signal_received:
        
        # React to sensor input
        sensor_signals = read_sensors()
        if sensor_signals.sound:
            interrupt_signal_received = True
            break
        if sensor_signals.obstacle or sensor_signals.touch:
            back_away_from_obstacle()            
            continue

        # Move random direction and distance
        rotation = random.randint(100,1000)
        distance = random.randint(1000,5000)
        movement_result = 0
        try:
            movement_result = subprocess.check_call([MOTORS_CMD, "FR"+str(rotation)])
        except:
            pass
        try:
            movement_result = subprocess.check_call([MOTORS_CMD, "FF"+str(distance)])
        except:
            pass
        
    sensor_daemon_proc.kill()

def back_away_from_obstacle():
    try:
        subprocess.check_call([RESET_SENSORS_CMD, "TO"])
        subprocess.check_call([MOTORS_CMD, "RR300"])
    except:
        pass
        
def read_sensors():
    results = { 'touch':0, 'obstacle':0, 'sound':0, 'range':0 }
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
    
def survey_surroundings():
    results = []
    img_file_prefix = datetime.datetime.now().strftime(IMG_FILE)
    for i in range(0, 9):
        img_file = img_file_prefix + str(i) + '.jpg'
        subprocess.call([MOTORS_CMD, PARTIAL_TURN])
        subprocess.call([PHOTO_CMD, img_file])
        sensor_signals = read_sensors()
        if sensor_signals.sound:
            interrupt_signal_received = True
            break
        results.append({ 'sensors':sensor_signals, 'image':img_file })
    return results
    
