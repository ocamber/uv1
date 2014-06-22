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
#include <sys/shm.h>
#include <unistd.h>
#include <wiringPi.h>
#include "gpio_pins.h"
#include "sensors.h"

void release_shared_memory(int shared_memory_id, SENSOR_DATA *sensor_values) {
	shmdt( (void *) sensor_values );
    shmctl( shared_memory_id, IPC_RMID, 0 );
}

int access_shared_memory(SENSOR_DATA *sensor_values, int ipc_mode) {
    // Set the shared memory key    (Shared memory key, Size in bytes, Permission flags)
    int shared_memory_id = shmget((key_t)SHARED_MEMORY_KEY, sizeof(SENSOR_DATA), ipc_mode);	
        //	Permission flags
        //		Operation permissions 	Octal value
        //		Read by user 			00400
        //		Write by user 			00200
        //		Read by group 			00040
        //		Write by group 			00020
        //		Read by others 			00004
        //		Write by others			00002
    if (shared_memory_id == -1)
    {
        return shared_memory_id;
    }
    
    //Make the shared memory accessible to the program
    sensor_values = (SENSOR_DATA *) shmat(shared_memory_id, (void *)0, 0);
    if (sensor_values == (SENSOR_DATA *) -1 )
    {
        if (ipc_mode & IPC_CREAT) {
            shmctl( shared_memory_id, IPC_RMID, 0 );
        }
        return -1;
    }
}

int access_sensors(SENSOR_DATA *sensor_values) {
    return access_shared_memory(sensor_values, 0);
}

int initialize_sensors(SENSOR_DATA *sensor_values) {

    /**
    * Shared memory initialization
    **/    

    int shared_memory_id = access_shared_memory(sensor_values, 0666 | IPC_CREAT);	
    
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
    pullUpDnControl(TOUCH_GPIO, PUD_DOWN);
    pinMode (OBSTACLE_GPIO, INPUT);
    pinMode (SOUND_GPIO, INPUT);
    pinMode (RANGE_ECHO_GPIO, INPUT);

    // Clear sensor values
    
    clear_sensor_values(sensor_values);
    if (write_sensor_file(sensor_values) <= 0) {
        release_shared_memory(shared_memory_id, sensor_values);
	    return -1;
	}

    return shared_memory_id;
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
	sensor_values->end_mark = SENSOR_DATA_END_MARK;
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
