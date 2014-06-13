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

static SENSOR_DATA sensor_values;

int main(int argc, char **argv)
{
    bool ok_args = (argc == 2);
    bool reset_touch = false;
    bool reset_obstacle = false;
    bool reset_sound = false;
    bool reset_range = false;
    
    if (ok_args)
    {
        char *arg = argv[1];
        int i;
        for (i=0; i < strlen(arg); i++) {
            char c = arg[i];
            if (c=='t' || c=='T') {
                if (reset_touch) {
                    ok_args = false;
                    break;
                }
                reset_touch = true;
            } else if (c=='o' || c=='O') {
                if (reset_obstacle) {
                    ok_args = false;
                    break;
                }
                reset_obstacle = true;
            } else if (c=='s' || c=='S') {
                if (reset_sound) {
                    ok_args = false;
                    break;
                }
                reset_sound = true;
            } else if (c=='r' || c=='R') {
                if (reset_range) {
                    ok_args = false;
                    break;
                }
                reset_range = true;
            } else {
                ok_args = false;
            }
        }
    }

    if (! ok_args)
    {
        fprintf(stderr, "Usage: reset_sensors [t|o|s|r]\n");
        fflush(stderr);
        exit(EXIT_FAILURE);
    }
    
    /**
    * Clear sensor values then read them from shared memory file
    */
    
    clear_sensor_values(&sensor_values);
    if (read_sensor_file(&sensor_values) < 0) {
        fprintf(stderr, "Cannot read sensor file!\n");
        fflush(stderr);
        exit(EXIT_FAILURE);
    }
    
    /**
    * Reset values per arg
    */
    
    if (reset_touch)
    {
        reset_touch_value(&sensor_values);
    }
    
    if (reset_obstacle)
    {
        reset_obstacle_value(&sensor_values);
    }
    
    if (reset_sound)
    {
        reset_sound_value(&sensor_values);
    }
    
    if (reset_range)
    {
        reset_range_value(&sensor_values);
    }
    
    /**
    * Write updated values to file
    */
    
    if (write_sensor_file(&sensor_values) < 0)
    {
        fprintf(stderr, "Cannot write sensor data to file!\n");
        fflush(stderr);
        exit(EXIT_FAILURE);
    }
    
    return EXIT_SUCCESS;

} // main
