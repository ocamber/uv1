/**
* reset_sensors.c - Reset sensor readings
*
* Oren Camber 2014-05-25
*
*/

#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "sensors.h"

static SENSOR_DATA *sensor_values;
static int shared_memory_id;

int main(int argc, char **argv)
{
    bool ok_args = (argc == 2);
    bool reset_range = false;
    bool reset_obstacle = false;
    bool reset_sound = false;
    
    if (ok_args)
    {
        char *arg = argv[1];
        int i;
        for (i=0; i < strlen(arg); i++) {
            char c = arg[i];
            switch (c) {
                case 'r':
                case 'R':
                    reset_range = true;
                    break;
                case 'o':
                case 'O':
                    reset_obstacle = true;
                    break;
                case 's':
                case 'S':
                    reset_sound = true;
                    break;
                default:
                ok_args = false;
            }
        }
    }

    if (! ok_args)
    {
        fprintf(stderr, "Usage: reset_sensors [r|o|s]\n");
        fflush(stderr);
        exit(EXIT_FAILURE);
    }
    
    // Access sensor memory - read-write without create
    shared_memory_id = access_sensor_memory( &sensor_values, 0 );	
    if (shared_memory_id < 0)
    {
        fprintf(stderr, "Cannot access sensor memory!\n");
        exit(EXIT_FAILURE);
    }
    
    /**
    * Clear sensor values then read them from shared memory file
    */
    
    clear_sensor_values(sensor_values);
    if (read_sensor_file(sensor_values) < 0) {
        fprintf(stderr, "Cannot read sensor file!\n");
        fflush(stderr);
        exit(EXIT_FAILURE);
    }
    
    /**
    * Reset values per arg
    */
    
    if (reset_range)
    {
        reset_range_value(sensor_values);
    }
    
    if (reset_obstacle)
    {
        reset_obstacle_value(sensor_values);
    }
    
    if (reset_sound)
    {
        reset_sound_value(sensor_values);
    }
    
    /**
    * Write updated values to file
    */
    
    if (write_sensor_file(sensor_values) < 0)
    {
        fprintf(stderr, "Cannot write sensor data to file!\n");
        fflush(stderr);
        exit(EXIT_FAILURE);
    }
    
    return EXIT_SUCCESS;

} // main
