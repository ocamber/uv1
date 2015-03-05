/**
* motors.c - Control uv1 left and right motors
* 
* Oren Camber 2014-05-21/**
* motors.c - Control uv1 left and right motors
* 
* compile with -lwiringPi
*/
 
#define SYNTAX_ERR  99
#define MOTORS_OFF "CC0"

#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/shm.h>
#include <wiringPi.h>
#include "gpio_pins.h"
#include "sensors.h"
#define HALT_ON_IMPACT      1
#define HALT_ON_OBSTACLE    2

static SENSOR_DATA *sensor_values;
static int halts;
static int shared_memory_id;

int execute_motion(char *);

bool motor_setting_err(char);
bool motion_syntax_err(char *);

int interrupted_duration = 0;

int main(int argc, char **argv)
{   
    // Test args
    bool bad_args = false;
    for (int i = 0; !bad_args && i < argc; i++) {
        if (strcmp("+i", argv[i]) == 0) {
            continue;
        }
        if (strcmp("-i", argv[i]) == 0) {
            continue;
        }
        if (strcmp("+o", argv[i]) == 0) {
            continue;
        }
        if (strcmp("-o", argv[i]) == 0) {
            continue;
        }
        bad_args = motion_syntax_err(argv[i]);
    }
    
    if (!bad_args) {
        printf("Usage: motors [-o | -i | +o | +i | {motion}]..\n\n");
        printf("Where: {motion} is 2 letters (one each or [F]wd, [R]ev, [B]rake, or [C]oast/Off,\n");
        printf("       followed by 4 digits for the duration in millisecs.\n");        
        printf("                            -or-\n");
        printf("       {filename} is the path to a file with motion entries as described above,\n");
        printf("       one entry per line.\n\n");
        printf("Args:  +i - Halt on impact (default).\n");
        printf("       -i - Execute motion even if sensors detect impact.\n");
        printf("       +o - Halt on obstacle detection (default).\n");
        printf("       -o - Execute motion even if sensors detect obstacle.\n\n\n");
        printf("Note:  Unless overridden motion will halt if a sensor detects obstacle or impact.\n");
        printf("       Motion will always halt if a sensor detects a sharp sound.\n");
        return SYNTAX_ERR;
    }
    
    // By default halt on anything 
    halts  = HALT_ON_IMPACT + HALT_ON_OBSTACLE;
    
    // Access sensor memory - read-write without create
    shared_memory_id = access_sensor_memory( &sensor_values, SHM_RDONLY );	
    if (shared_memory_id < 0)
    {
        fprintf(stderr, "Cannot access sensor memory!\n");
        exit(EXIT_FAILURE);
    }
    
    // Set up GPIO pins
    wiringPiSetupGpio();
    pinMode (LEFT_MOTOR_FWD_GPIO, OUTPUT);
    pinMode (LEFT_MOTOR_REV_GPIO, OUTPUT);
    pinMode (RIGHT_MOTOR_FWD_GPIO, OUTPUT);
    pinMode (RIGHT_MOTOR_REV_GPIO, OUTPUT);
    
    for (int i = 0; i < argc; i++) {
        if (strcmp("+i", argv[i]) == 0) {
            halts |= HALT_ON_IMPACT;
            continue;
        }
        if (strcmp("-i", argv[i]) == 0) {
            halts &= ~HALT_ON_IMPACT;
            continue;
        }
        if (strcmp("+o", argv[i]) == 0) {
            halts |= HALT_ON_OBSTACLE;
            continue;
        }
        if (strcmp("-o", argv[i]) == 0) {
            halts &= ~HALT_ON_OBSTACLE;
            continue;
        }
        
        interrupted_duration = execute_motion(argv[1]);
        
        printf( "%s -%d\n", argv[i], interrupted_duration );
        if (interrupted_duration) {
            break;
        }
    }
    
    execute_motion(MOTORS_OFF);
    
    return interrupted_duration;

} // main

bool motor_setting_err(char setting) {
    switch (setting)
    {
        case 'F':   // Forward
        case 'f':
        case 'R':   // Reverse
        case 'r':
        case 'B':   // Brake
        case 'b':
        case 'C':   // Coast
        case 'c':
            return false;
    }
    return true;
}

bool motion_syntax_err(char *motion) {
    int test_value;
    if (motion_indicator_err(motion[0])) {  // Left motor
        return false;
    }
    if (motion_indicator_err(motion[1])) {  // Right motor
        return false;
    }
    if (sscanf( (motion + 2), "%d", &test_value ) < 1) {
        return false;
    }
    return true;
}

int execute_motion(char *motion)
{
    char left_motion = motion[0];
    char right_motion = motion[1];
    
    int remaining_duration = 0;

    sscanf( (motion + 2), "%d", &remaining_duration );

    switch (left_motion)
    {
        case 'F':        //  Left forward
        case 'f':
            left_motion = 'F';
            digitalWrite(LEFT_MOTOR_FWD_GPIO, HIGH);
            digitalWrite(LEFT_MOTOR_REV_GPIO, LOW);
            break;
        case 'R':        // Left reverse
        case 'r':
            left_motion = 'R';
            digitalWrite(LEFT_MOTOR_FWD_GPIO, LOW);
            digitalWrite(LEFT_MOTOR_REV_GPIO, HIGH);
            break;
        case 'B':        // Left brake
        case 'b':
            left_motion = 'B';
            digitalWrite(LEFT_MOTOR_FWD_GPIO, HIGH);
            digitalWrite(LEFT_MOTOR_REV_GPIO, HIGH);
            break;
        case 'C':        // Left coast / off
        case 'c':
            left_motion = 'C';
            digitalWrite(LEFT_MOTOR_FWD_GPIO, LOW);
            digitalWrite(LEFT_MOTOR_REV_GPIO, LOW);
            break;
        default:
            break;
    }

    switch (right_motion)
    {
        case 'F':        // Right forward
        case 'f':
            right_motion = 'F';
            digitalWrite(RIGHT_MOTOR_FWD_GPIO, HIGH);
            digitalWrite(RIGHT_MOTOR_REV_GPIO, LOW);
            break;
        case 'R':        // Right reverse
        case 'r':
            right_motion = 'R';
            digitalWrite(RIGHT_MOTOR_FWD_GPIO, LOW);
            digitalWrite(RIGHT_MOTOR_REV_GPIO, HIGH);
            break;
        case 'B':        // Right brake
        case 'b':
            right_motion = 'B';
            digitalWrite(RIGHT_MOTOR_FWD_GPIO, HIGH);
            digitalWrite(RIGHT_MOTOR_REV_GPIO, HIGH);
            break;
        case 'C':        // Right coast / off
        case 'c':
            right_motion = 'C';
            digitalWrite(RIGHT_MOTOR_FWD_GPIO, LOW);
            digitalWrite(RIGHT_MOTOR_REV_GPIO, LOW);
            break;
        default:
            break;
    }
    
    while (remaining_duration > 0)
    {
        if (sensor_values->sound_val == POSITIVE_VAL)
        {
            break;
        }
        
        if (sensor_values->impact_val[IDX_FWD] == POSITIVE_VAL
            && (left_motion=='F' || right_motion=='F')
            && (halts & HALT_ON_IMPACT))
        {
            break;
        }
        
        if (sensor_values->impact_val[IDX_BACK] == POSITIVE_VAL
            && (left_motion=='R' || right_motion=='R')
            && (halts & HALT_ON_IMPACT))
        {
            break;
        }
        
        if (left_motion=='F' 
            && (sensor_values->obstacle_val[IDX_FWD] == POSITIVE_VAL
                || sensor_values->obstacle_val[IDX_RIGHT] == POSITIVE_VAL)
            && (halts & HALT_ON_OBSTACLE))
        {
            break;
        }
            
        if (right_motion=='F'
            && (sensor_values->obstacle_val[IDX_FWD] == POSITIVE_VAL
                || sensor_values->obstacle_val[IDX_LEFT] == POSITIVE_VAL)
            && (halts & HALT_ON_OBSTACLE))
        {
            break;
        }

        if ((left_motion=='R' || right_motion=='R')
            && sensor_values->obstacle_val[IDX_BACK] == POSITIVE_VAL
            && (halts & HALT_ON_OBSTACLE))
        {
            break;
        }

        remaining_duration --;
        delay(1);
    }
    return remaining_duration;
}

