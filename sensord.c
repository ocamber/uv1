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
#include <sys/shm.h>
#include <sys/time.h>
#include <wiringPi.h>
#include "gpio_pins.h"
#include "sensors.h"

#define SPEED_OF_SOUND      0.0000343  // cm/nanosecond

void terminate_signal_handler(int sig);

void range_echo_handler(void);
void obstacle_f_handler(void);
void obstacle_l_handler(void);
void obstacle_r_handler(void);
void obstacle_b_handler(void);
void sound_handler(void);
void impact_f_handler(void);
void impact_b_handler(void);
void set_positive(char, char *, char *, int);

static struct timespec echo_start;      // Start time of range echo signal 
        // Rangefinder sets pin HIGH for the time it took the pulse to leave and return as echo

static SENSOR_DATA *sensor_values;
static int shared_memory_id;

static volatile bool TERMINATE_SIGNAL_RECEIVED = false;

int main(void) {
    
    // Register signal handlers for graceful termination
    
    if (signal(SIGINT, terminate_signal_handler) == SIG_ERR) {
        fprintf(stderr, "Cannot handle SIGINT!\n");
        exit(EXIT_FAILURE);
    }
    
    if (signal(SIGTERM, terminate_signal_handler) == SIG_ERR) {
        fprintf(stderr, "Cannot handle SIGTERM!\n");
        exit(EXIT_FAILURE);
    }
    
    if (signal(SIGHUP, terminate_signal_handler) == SIG_ERR) {
        fprintf(stderr, "Cannot handle SIGHUP!\n");
        exit(EXIT_FAILURE);
    }
    
    if (signal(SIGQUIT, terminate_signal_handler) == SIG_ERR) {
        fprintf(stderr, "Cannot handle SIGQUIT!\n");
        exit(EXIT_FAILURE);
    }
    
    if (signal(SIGABRT, terminate_signal_handler) == SIG_ERR) {
        fprintf(stderr, "Cannot handle SIGABRT!\n");
        exit(EXIT_FAILURE);
    }

    // Sensor data setup

    /**
    * Shared memory initialization
    **/    

    shared_memory_id = access_sensor_memory( &sensor_values, (0666 | IPC_CREAT) );	
    if (shared_memory_id < 0)
    {
        fprintf(stderr, "Cannot access sensor memory!\n");
        exit(EXIT_FAILURE);
    }
    
    /**
    * WiringPi and GPIO initialization
    **/

    wiringPiSetupGpio();

    // Output pins

    pinMode (RANGE_TRIGGER_GPIO, OUTPUT);
    pinMode (LEFT_MOTOR_FWD_GPIO, OUTPUT);
    pinMode (LEFT_MOTOR_REV_GPIO, OUTPUT);
    pinMode (RIGHT_MOTOR_FWD_GPIO, OUTPUT);
    pinMode (RIGHT_MOTOR_REV_GPIO, OUTPUT);

    // Input pins

    pinMode (OBSTACLE_B_GPIO, INPUT);
    pinMode (OBSTACLE_F_GPIO, INPUT);
    pinMode (IMPACT_B_GPIO, INPUT);
    pinMode (IMPACT_F_GPIO, INPUT);
    pinMode (OBSTACLE_L_GPIO, INPUT);
    pinMode (OBSTACLE_R_GPIO, INPUT);
    pinMode (SOUND_GPIO, INPUT);
    pinMode (RANGE_ECHO_GPIO, INPUT);

    // Clear sensor values
    
    clear_sensor_values(sensor_values);
    if (write_sensor_file(sensor_values) <= 0) {
        release_sensor_memory(shared_memory_id, sensor_values);
        fprintf(stderr, "Cannot clear sensor values!\n");
        exit(EXIT_FAILURE);
	}

    // GPIO signal handlers

    wiringPiISR(RANGE_ECHO_GPIO, INT_EDGE_BOTH, &range_echo_handler);
    wiringPiISR(IMPACT_F_GPIO, INT_EDGE_RISING, &impact_f_handler);
    wiringPiISR(IMPACT_B_GPIO, INT_EDGE_RISING, &impact_b_handler);
    wiringPiISR(OBSTACLE_F_GPIO, INT_EDGE_FALLING, &obstacle_f_handler);
    wiringPiISR(OBSTACLE_B_GPIO, INT_EDGE_FALLING, &obstacle_b_handler);
    wiringPiISR(OBSTACLE_L_GPIO, INT_EDGE_FALLING, &obstacle_l_handler);
    wiringPiISR(OBSTACLE_R_GPIO, INT_EDGE_FALLING, &obstacle_r_handler);
    wiringPiISR(SOUND_GPIO, INT_EDGE_FALLING, &sound_handler);

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
        sensor_values->range_indic = NO_RANGE_INDICATOR;
        // Send 10 usec pulse
        digitalWrite(RANGE_TRIGGER_GPIO, HIGH);
        nanosleep(&pulse_width, (struct timespec *)NULL);
        digitalWrite(RANGE_TRIGGER_GPIO, LOW);
        
        // Wait for any reading (echo_handler will process signals)
        nanosleep(&max_echo_time, (struct timespec *)NULL);
        
        // If there was no reading, set range to 999 and write file
        if (sensor_values->range_indic != RANGE_INDICATOR) {
            sensor_values->range_val[0] = '9';   // Hundreds
            sensor_values->range_val[1] = '9';   // Tens
            sensor_values->range_val[2] = '9';   // Ones
            sensor_values->range_indic = RANGE_INDICATOR;
            write_sensor_file(sensor_values);
        }
        
        // Wait remainder of time until next pulse
        nanosleep(&inter_pulse_interval, (struct timespec *)NULL);
        
        // Read any sensor values updated by external program without shared memory
        read_sensor_file(sensor_values);
    }
    
    // Before termination, turn off rangefinder
    digitalWrite(RANGE_TRIGGER_GPIO, LOW); 
    
    // Detach and delete shared memory
    release_sensor_memory(shared_memory_id, sensor_values);
           
    exit(EXIT_SUCCESS);
}    

void set_positive(char indicator, char *indic_ptr, char *values, int direction) {
    // If sensor value is already positive, exit here
    if (*indic_ptr == indicator && values[direction] == POSITIVE_VAL) {
        return;
    }
    // Otherwise update shared memory and write file
    *indic_ptr = indicator;
    values[direction] = POSITIVE_VAL; 
    write_sensor_file(sensor_values);
}

void impact_f_handler() {
    set_positive(IMPACT_INDICATOR, &sensor_values->impact_indic, 
                    (char *) &sensor_values->impact_val, IDX_FWD);
}

void impact_b_handler() {
    set_positive(IMPACT_INDICATOR, &sensor_values->impact_indic, 
                    (char *) &sensor_values->impact_val, IDX_BACK);
}

void obstacle_f_handler() {
    set_positive(OBSTACLE_INDICATOR, &sensor_values->obstacle_indic, 
                    (char *) &sensor_values->obstacle_val, IDX_FWD);
}

void obstacle_b_handler() {
    set_positive(OBSTACLE_INDICATOR, &sensor_values->obstacle_indic, 
                    (char *) &sensor_values->obstacle_val, IDX_BACK);
}

void obstacle_l_handler() {
    set_positive(OBSTACLE_INDICATOR, &sensor_values->obstacle_indic, 
                    (char *) &sensor_values->obstacle_val, IDX_LEFT);
}

void obstacle_r_handler() {
    set_positive(OBSTACLE_INDICATOR, &sensor_values->obstacle_indic, 
                    (char *) &sensor_values->obstacle_val, IDX_RIGHT);
}

void sound_handler() {
    set_positive(SOUND_INDICATOR, &sensor_values->sound_indic, 
                    &sensor_values->sound_val, 0);
}

void range_echo_handler() {
    if (sensor_values->range_indic == RANGE_INDICATOR) {
        return;         // If not waiting for measurement, exit here
    }
    
    int pin_value = digitalRead(RANGE_ECHO_GPIO);
    
    // Handle start of echo signal
    if (sensor_values->range_indic == NO_RANGE_INDICATOR && pin_value == HIGH) {
        clock_gettime(CLOCK_REALTIME, &echo_start);
        sensor_values->range_indic = RANGE_INDICATOR;
        return;
    }
    
    if ( ! (sensor_values->range_indic == RANGE_INDICATOR && pin_value == LOW) ) {
        return;
    }
    
    // Handle end of echo signal
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    
    int range = 0;
    long secs = now.tv_sec - echo_start.tv_sec;       // Get seconds
    long nsecs = now.tv_nsec - echo_start.tv_nsec;    // Get nanoseconds (may be negative if second has changed!)
    if (secs > 1) {
        range = 999;    // If time includes whole seconds, just set range to MAXIMUM
    } else if (secs < 0) {
        range = 0;      // If time is negative, set range to 0 - Should never happen!
    } else {
        long t = (1000000000L * secs) + nsecs;      // Time (nsec) for pulse echo to return
        double r = (double) SPEED_OF_SOUND * (double) t / 2;   // Range in cm
        if (r < 0) {                                // Make integer from 0-999 cm
            range = 0;
        } else if (r > 999) {
            range = 999;
        } else {
            range = (int) (r + 0.5);
        }
    }
    sensor_values->range_val[0] = '0' + (range / 100);       // Hundreds
    sensor_values->range_val[1] = '0' + ((range / 10) % 10); // Tens
    sensor_values->range_val[2] = '0' + (range % 10);        // Ones
    sensor_values->range_indic = RANGE_INDICATOR;
    write_sensor_file(sensor_values);
}

void terminate_signal_handler(int sig) {
    TERMINATE_SIGNAL_RECEIVED = true;
}
