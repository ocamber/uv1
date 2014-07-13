#!/usr/bin/python
# uv1-simple.py
# Automated simple main procedure for RPI-UV1 drone.
#
# Author : Oren Camber
# Date   : 2014-07-11

# Import required Python libraries
import random
import subprocess
import datetime
import RPi.GPIO as GPIO

IMG_FILE = '/home/pi/UV1-IMG-%Y%m%d%H%M%S-'
SENSOR_FILE = '/dev/shm/sensor_data'
SENSORD_CMD = '/home/pi/src/uv1/sensord'
MOTORS_CMD = '/home/pi/src/uv1/motors'
LIGHTS_CMD = '/home/pi/src/uv1/lights'
LASER_CMD = '/home/pi/src/uv1/laser'
RESET_SENSORS_CMD = '/home/pi/src/uv1/reset_sensors'
PHOTO_CMD = 'raspistill'
VIDEO_CMD = 'raspivid'
LIGHTS_GPIO = 14
LASER_GPIO  = 7
MOTOR_CORRECTION = "FC"
CORRECTION_RATIO = 0.1
CORRECTION_INTERVAL = 500
MOTOR_DEG = 5.83 
MOTOR_CM = 52.5
PROXIMITY_SENSORS = "TO"
PARTIAL_TURN = "FR210"
BACK_AWAY = "RR200"


# Use BCM GPIO references instead of physical pin numbers
GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)
GPIO.setup(LIGHTS_GPIO, GPIO.OUT)
GPIO.setup(LASER_GPIO, GPIO.OUT)
GPIO.output(LIGHTS_GPIO, False)
GPIO.output(LASER_GPIO, False)

def main(): 

    sensor_signals = None
    sensor_daemon_proc = subprocess.Popen([SENSORD_CMD])
    
    # Initialize randomizer
    random.seed()
    
    while True:
        
        # React to sensor input
        sensor_signals = read_sensors()
        if sensor_signals['sound']:
            break
        if sensor_signals['obstacle'] or sensor_signals['touch']:
            back_away_from_obstacle()            
            continue
            
        # Move random direction and distance (if range < 10 cm turn a lot)
        if sensor_signals['range'] < 10:
            rotation = random.randint(90,180)
        else:
            rotation = random.randint(30,230)            
        distance = random.randint(50,300)
        movement_result = proceed(rotation, distance)
        data = survey_surroundings()
                         
    sensor_daemon_proc.kill()
        
def proceed(rotation, distance):
    # convert degrees and cm to motor control values
    rot_value = int(rotation * MOTOR_DEG + 0.5)
    dist_value = int(distance * MOTOR_CM + 0.5)
    try:
        movement_result = subprocess.check_call([MOTORS_CMD, "FR"+str(rot_value)])
    except:
        pass    
    remainder = dist_value
    while remainder > 0:
        interval = remainder
        if interval > CORRECTION_INTERVAL:
            interval = CORRECTION_INTERVAL
        try:
            movement_result = subprocess.check_call([MOTORS_CMD, "FF"+str(interval)])
        except:
            pass
        remainder = remainder - interval;
        correction = int(CORRECTION_RATIO * interval)
        try:
            movement_result = subprocess.check_call([MOTORS_CMD, MOTOR_CORRECTION+str(correction)])
        except:
            pass
        
    
def back_away_from_obstacle():
    try:
        subprocess.check_call([RESET_SENSORS_CMD, PROXIMITY_SENSORS])
        subprocess.check_call([MOTORS_CMD, BACK_AWAY])
    except:
        pass
        
def read_sensors():
    results = { 'touch':0, 'obstacle':0, 'sound':0, 'range':0 }
    with open(SENSOR_FILE, 'r') as file:
        file_text = file.read()
    if file_text[0:1]=='T' and file_text[1:2]=='+':
        results['touch'] = 1
    if file_text[2:3]=='O' and file_text[3:4]=='+':
        results['obstacle'] = 1
    if file_text[4:5]=='S' and file_text[5:6]=='+':
        results['sound'] = 1
    if file_text[6:7]=='R':
        results['range'] = int(file_text[7:10], base=10)
    return results
    
def survey_surroundings():
    results = []
    img_file_prefix = datetime.datetime.now().strftime(IMG_FILE)
    for i in range(0, 10):
        img_file = img_file_prefix + str(i) + '.jpg'
        try:
            subprocess.check_call([MOTORS_CMD, PARTIAL_TURN])
            subprocess.check_call([PHOTO_CMD, "-n", "-o", img_file])
        except:
            pass
        sensor_signals = read_sensors()
        results.append({ 'sensors':sensor_signals, 'image':img_file })
    return results

main()
