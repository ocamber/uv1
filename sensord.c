/**
* sensord.c - Sensor daemon service - reads sensor readings
* and updates sensor_data shared memory file
*
* Oren Camber 2014-05-25
*
* compile with sensors.o + -lwiringPi
*/

#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <wiringPi.h>
#include "gpio_pins.h"
#include "sensors.h"

#define INITIALIZE_DATA     true
#define SPEED_OF_SOUND      0.00000343  // cm/nanosecond

void terminate_signal_handler(int sig);

void stop_motors(void);
void touch_handler(void);
void obstacle_handler(void);
void sound_handler(void);
void range_echo_handler(void);

static struct timespec range_pulse_sent;
static struct timespec echo_start;      // Start time of range echo signal 
                                        // Rangefinder sets pin HIGH for same time it took echo to return

static SENSOR_DATA sensor_values;
static bool TERMINATE_SIGNAL_RECEIVED = false;

int main(void) {
    
    // Register signal handlers for graceful termination
    
    if (signal(SIGINT, terminate_signal_handler) == SIG_ERR) {
        fprintf(stderr, "Cannot handle SIGINT!\n");
        fflush(stderr);
        exit(EXIT_FAILURE);
    }
    
    if (signal(SIGTERM, terminate_signal_handler) == SIG_ERR) {
        fprintf(stderr, "Cannot handle SIGTERM!\n");
        fflush(stderr);
        exit(EXIT_FAILURE);
    }
    
    if (signal(SIGHUP, terminate_signal_handler) == SIG_ERR) {
        fprintf(stderr, "Cannot handle SIGHUP!\n");
        fflush(stderr);
        exit(EXIT_FAILURE);
    }
    
    if (signal(SIGQUIT, terminate_signal_handler) == SIG_ERR) {
        fprintf(stderr, "Cannot handle SIGQUIT!\n");
        fflush(stderr);
        exit(EXIT_FAILURE);
    }
    
    if (signal(SIGABRT, terminate_signal_handler) == SIG_ERR) {
        fprintf(stderr, "Cannot handle SIGABRT!\n");
        fflush(stderr);
        exit(EXIT_FAILURE);
    }

    // Sensor data setup

    if (!open_sensors(&sensor_values, INITIALIZE_DATA)) {
        fprintf(stderr, "Cannot set up sensor_data file!\n");
        fflush(stderr);
        exit(EXIT_FAILURE);
    }

    // GPIO signal handlers

    wiringPiISR(TOUCH_GPIO, INT_EDGE_RISING, &touch_handler);
    wiringPiISR(OBSTACLE_GPIO, INT_EDGE_FALLING, &obstacle_handler);
    wiringPiISR(SOUND_GPIO, INT_EDGE_FALLING, &sound_handler);
    wiringPiISR(RANGE_ECHO_GPIO, INT_EDGE_BOTH, &range_echo_handler);

    // Range finder scan loop

    struct timespec pulse_width;            // Pulse width is 10 usec
    pulse_width.tv_sec = 0;
    pulse_width.tv_nsec = 10000L;

    struct timespec max_echo_time;          // Wait for end of echo signal up to 100 msec
    max_echo_time.tv_sec = 0;
    max_echo_time.tv_nsec = 100000000L;

    struct timespec inter_pulse_interval;   // Pulses sent every 200 msec
    inter_pulse_interval.tv_sec = 0;
    inter_pulse_interval.tv_nsec = 200000000L - (pulse_width.tv_nsec + max_echo_time.tv_nsec);

    while(!TERMINATE_SIGNAL_RECEIVED) {
        sensor_values.range = NO_RANGE_INDICATOR;
        // Send 10 usec pulse
        digitalWrite(RANGE_TRIGGER_GPIO, HIGH);
        nanosleep(&pulse_width, (struct timespec *)NULL);
        digitalWrite(RANGE_TRIGGER_GPIO, LOW);
        
        // Wait for any reading (echo_handler will process signals)
        nanosleep(&max_echo_time, (struct timespec *)NULL);
        
        // If there was no reading, set range to 999 and write file
        if (sensor_values.range == NO_RANGE_INDICATOR) {
            sensor_values.range_val[0] = '9';   // Hundreds
            sensor_values.range_val[1] = '9';   // Tens
            sensor_values.range_val[2] = '9';   // Ones
            sensor_values.range = RANGE_INDICATOR;
            write_sensor_file(&sensor_values);
        }
        
        // Wait remainder of time until next pulse
        nanosleep(&inter_pulse_interval, (struct timespec *)NULL);
        
        // Read any sensor values updated by external program
        read_sensor_file(&sensor_values);
    }
    
    // Before termination, turn off rangefinder
    digitalWrite(RANGE_TRIGGER_GPIO, LOW); 
           
    exit(EXIT_SUCCESS);
}    

void touch_handler() {
    stop_motors();      // Reflex action - stop motors
    
    if (sensor_values.touch == TOUCH_INDICATOR) {
        return;         // If a touch was already detected, exit here
    }
    sensor_values.touch_val = POSITIVE_VAL;
    sensor_values.touch = TOUCH_INDICATOR;
    write_sensor_file(&sensor_values);
}

void obstacle_handler() {
    stop_motors();      // Reflex action - stop motors
    
    if (sensor_values.obstacle == OBSTACLE_INDICATOR) {
        return;         // If an obstacle was already detected, exit here
    }
    sensor_values.obstacle_val = POSITIVE_VAL;
    sensor_values.obstacle = OBSTACLE_INDICATOR;
    write_sensor_file(&sensor_values);
}

void sound_handler() {
    stop_motors();      // Reflex action - stop motors
    if (sensor_values.sound == SOUND_INDICATOR) {
        return;         // If a sound was already detected, exit here
    }
    sensor_values.sound_val = POSITIVE_VAL;
    sensor_values.sound = SOUND_INDICATOR;
    write_sensor_file(&sensor_values);
}

void range_echo_handler() {
    if (sensor_values.range != NO_RANGE_INDICATOR) {
        return;         // If not waiting for measurement, exit here
    }
    
    int pin_value = digitalRead(RANGE_ECHO_GPIO);
    
    // Handle start of echo signal
    if (pin_value == HIGH) {
        clock_gettime(CLOCK_REALTIME, &echo_start);
        return;
    }
    
    // Handle end of echo signal
    int range = 0;
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    long secs = now.tv_sec - echo_start.tv_sec;       // Get seconds
    long nsecs = now.tv_nsec - echo_start.tv_nsec;    // Get nanoseconds (may be negative if second has changed!)
    if (secs > 1) {
        range = 999;    // If time includes whole seconds, just set range to MAXIMUM
    } else if (secs < 0) {
        range = 0;      // If time is negative, set range to 0 - Should never happen!
    } else {
        long t = (1000000000L * secs) + nsecs;      // Time (nsec) for pulse echo to return
        float r = (float) SPEED_OF_SOUND * t / 2;   // Range in cm
        if (r < 0) {                                // Make integer from 0-999 cm
            range = 0;
        } else if (r > 999) {
            range = 999;
        } else {
            range = (int) r;
        }
    }
    sensor_values.range_val[0] = '0' + (range / 100);       // Hundreds
    sensor_values.range_val[1] = '0' + ((range / 10) % 10); // Tens
    sensor_values.range_val[2] = '0' + (range % 10);        // Ones
    sensor_values.range = RANGE_INDICATOR;
    write_sensor_file(&sensor_values);
}

void stop_motors() {
    digitalWrite(LEFT_MOTOR_FWD_GPIO, LOW);
    digitalWrite(LEFT_MOTOR_REV_GPIO, LOW);
    digitalWrite(RIGHT_MOTOR_FWD_GPIO, LOW);
    digitalWrite(RIGHT_MOTOR_REV_GPIO, LOW);    
}

void terminate_signal_handler(int sig) {
    TERMINATE_SIGNAL_RECEIVED = true;
}
