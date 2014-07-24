#!/usr/bin/python
# uv1-simple.py
# Simple automated exploration procedure for RPI-UV1 drone.
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
RESET_SENSORS_CMD = '/home/pi/src/uv1/reset_sensors'
MOTORS_CMD = '/home/pi/src/uv1/motors'
LIGHTS_CMD = '/home/pi/src/uv1/lights'
LASER_CMD = '/home/pi/src/uv1/laser'
PHOTO_CMD = 'raspistill'
VIDEO_CMD = 'raspivid'
LIGHTS_GPIO = 14
LASER_GPIO  = 7
FWD_MOTOR_CORRECTION = "FC"
RVS_MOTOR_CORRECTION = "CF"
MOTOR_CORRECTION_RATIO = 0.05
MAX_MOTOR_INTERVAL_CM = 20
MOTOR_MS_PER_DEG = 5.83
MOTOR_MS_PER_CM = 52.5
PROXIMITY_SENSORS = "TO"
PARTIAL_TURN = "FR190"
BACK_AWAY = "RR200"


# Use BCM GPIO references instead of physical pin numbers
GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)
GPIO.setup(LIGHTS_GPIO, GPIO.OUT)
GPIO.setup(LASER_GPIO, GPIO.OUT)
GPIO.output(LIGHTS_GPIO, False)
GPIO.output(LASER_GPIO, False)


def main():

    # Initialize randomizer and sensor readings
    random.seed()
    sensor_signals = None

    while True:

        # React to sensor input
        sensor_signals = read_sensors()
        if sensor_signals['sound']:
            break
        if sensor_signals['obstacle'] or sensor_signals['touch']:
            back_away_from_obstacle()
            rotate_to_avoid_obstacle()
            continue
        if sensor_signals['range'] < 10:
            rotate_to_avoid_obstacle()
            continue
            
        # Periodically scan surroundings
        if random.randint(0,20)<1:
            survey_surroundings()

        # Periodically rotate to new direction
        if random.randint(0,10)<2:
            rotate(random.randint(0,90) - 45)
            continue

        # Go forward random cm to a new position
        go_forward(random.randint(0,MAX_MOTOR_INTERVAL_CM) + 10)

def rotate(degrees):
    if not degrees:
        return

    # convert degrees to motors / milliseconds
    ms = int(abs(degrees) * MOTOR_MS_PER_DEG + 0.5)
    motors = "FR"
    if degrees < 0:
        motors = "RF"
    movement_result = subprocess.call([MOTORS_CMD, motors+str(ms)])
    log_motion(movement_result)
    return movement_result

def go_forward(cm):
    if not cm:
        return

    # convert cm to motors / milliseconds
    ms = int(abs(cm) * MOTOR_MS_PER_CM + 0.5)
    correction = "FC"
    motors = "FF"
    if cm < 0:
        motors = "RR"
        movement_result = subprocess.call([MOTORS_CMD, motors+str(ms)])
        motors = "CF"
        subprocess.call([MOTORS_CMD, motors+str(ms)])
    else:
        motors = "FF"
        movement_result = subprocess.call([MOTORS_CMD, motors+str(ms)])
        motors = "FC"
        subprocess.call([MOTORS_CMD, motors+str(ms)])

    # Execute turn to correct straightness
    subprocess.call([MOTORS_CMD, motors+str(int(MOTOR_CORRECTION_RATIO * ms))])
    log_motion(movement_result)
    return movement_result

def back_away_from_obstacle():
    subprocess.call([RESET_SENSORS_CMD, PROXIMITY_SENSORS])
    movement_result = subprocess.call([MOTORS_CMD, BACK_AWAY])
    subprocess.call([RESET_SENSORS_CMD, PROXIMITY_SENSORS])
    log_motion(movement_result)
    return movement_result

def rotate_to_avoid_obstacle():
    subprocess.call([RESET_SENSORS_CMD, PROXIMITY_SENSORS])
    movement_result = rotate(random.randint(90,180))
    subprocess.call([RESET_SENSORS_CMD, PROXIMITY_SENSORS])
    log_motion(movement_result)
    return movement_result

def read_sensors():
    reading = { 'touch':0, 'obstacle':0, 'sound':0, 'range':0 }
    with open(SENSOR_FILE, 'r') as file:
        file_text = file.read()
    if file_text[0:1]=='T' and file_text[1:2]=='+':
        reading['touch'] = 1
    if file_text[2:3]=='O' and file_text[3:4]=='+':
        reading['obstacle'] = 1
    if file_text[4:5]=='S' and file_text[5:6]=='+':
        reading['sound'] = 1
    if file_text[6:7]=='R':
        reading['range'] = int(file_text[7:10], base=10)
    return reading

def survey_surroundings():
    survey = []
    img_group = datetime.datetime.now().strftime(IMG_FILE)
    for i in range(0, 10):
        sensor_signals = read_sensors()
        img_file = img_group + str(i) + '.jpg'
        subprocess.call([PHOTO_CMD, "-n", "-o", img_file])
        survey.append({ 'sensors':sensor_signals, 'image':img_file })
        if sensor_signals['sound'] or sensor_signals['touch']:
            break
        subprocess.call([MOTORS_CMD, PARTIAL_TURN])
    log_survey(survey)
    return survey

def log_survey(survey):
    pass

def log_motion(movement_result):
    pass
    
main()