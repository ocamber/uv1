#!/usr/bin/python
# uv1.py
# Main procedure for RPI-UV1 drone.
#
# Author : Oren Camber
# Date   : 2014-06-21

# Import required Python libraries
import subprocess
import time
import RPi.GPIO as GPIO

# Use BCM GPIO references
# instead of physical pin numbers
GPIO.setmode(GPIO.BCM)

SENSOR_FILE = '/dev/shm/sensor_data'
SENSORD_CMD = '/home/pi/src/uv1/sensord'
MOTORS_CMD = '/home/pi/src/uv1/motors'
LIGHTS_CMD = '/home/pi/src/uv1/lights'
LASER_CMD = '/home/pi/src/uv1/laser'
RESET_SENSORS_CMD = '/home/pi/src/uv1/reset_sensors'

sensor_daemon_proc = None

def main(args):
    sensor_daemon_proc = subprocess.Popen([SENSORD_CMD])
