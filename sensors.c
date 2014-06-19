/**
* sensors.c - Sensor data access
* 
* Oren Camber 2014-05-23
*
*/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <wiringPi.h>
#include "gpio_pins.h"
#include "sensors.h"

bool open_sensors(SENSOR_DATA *sensor_values) {
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

    pinMode (TOUCH_GPIO, INPUT);
    pullUpDnControl (TOUCH_GPIO, PUD_UP);
    pinMode (OBSTACLE_GPIO, INPUT);
    pinMode (SOUND_GPIO, INPUT);
    pinMode (RANGE_ECHO_GPIO, INPUT);

    // Clear sensor values
    
    clear_sensor_values(sensor_values);
    if (write_sensor_file(sensor_values) <= 0) {
	    return false;
	}

	return true;
}

size_t read_sensor_file(SENSOR_DATA *sensor_values) {
    FILE *sensor_file = fopen(SENSOR_FILE, "r"); // Open read only
    if (sensor_file == NULL) 
    {
        return -1;
    }
    size_t result = fread(sensor_values, sizeof(SENSOR_DATA), 1, sensor_file);
    fclose(sensor_file);
    return result;
}

size_t write_sensor_file(SENSOR_DATA *sensor_values) {
    FILE *sensor_file = fopen(SENSOR_FILE, "w");
    size_t result = fwrite(sensor_values, sizeof(SENSOR_DATA), 1, sensor_file);
    fclose(sensor_file);
    return result;    
}

void clear_sensor_values(SENSOR_DATA *sensor_values) {
	reset_touch_value(sensor_values);
	reset_obstacle_value(sensor_values);
	reset_sound_value(sensor_values);
    reset_range_value(sensor_values);
	sensor_values->end_mark = '\n';
}

void reset_touch_value(SENSOR_DATA *sensor_values) {
	sensor_values->touch = NO_TOUCH_INDICATOR;
	sensor_values->touch_val = NEGATIVE_VAL;
}

void reset_obstacle_value(SENSOR_DATA *sensor_values) {
	sensor_values->obstacle = NO_OBSTACLE_INDICATOR;
	sensor_values->obstacle_val = NEGATIVE_VAL;
}

void reset_sound_value(SENSOR_DATA *sensor_values) {
	sensor_values->sound = NO_SOUND_INDICATOR;
	sensor_values->sound_val = NEGATIVE_VAL;
}

void reset_range_value(SENSOR_DATA *sensor_values) {
	sensor_values->range = NO_RANGE_INDICATOR;
	sensor_values->range_val[0] = '0';
	sensor_values->range_val[1] = '0';
	sensor_values->range_val[2] = '0';
}
