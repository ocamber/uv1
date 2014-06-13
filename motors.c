/**
* motors.c - Control uv1 left and right motors
* 
* Oren Camber 2014-05-21/**
* motors.c - Control uv1 left and right motors
* 
* compile with -lwiringPi
*/
 
#define MOTORS_OFF "CC0"

#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <wiringPi.h>
#include "gpio_pins.h"

FILE *stream;
char data[100];

void execute_motion(char *);

int main(int argc, char **argv)
{
    if (argc < 2 || argc > 3 || (argc==3 && strcmp("-f", argv[1]) != 0 ))
    {
        printf("Usage: motors {motion}  -or-  motors -f {filename}\n\n");
        printf("Where: {motion} is 2 letters (one each or [F]wd, [R]ev, [B]rake, or [C]oast/Off,\n");
        printf("       followed by 4 digits for the duration in millisecs.\n");        
        printf("                            -or-\n");
        printf("       {filename} is the path to a file with motion entries as described above,\n");
        printf("       one entry per line.\n\n");
        printf("Note:  Motors will be halted if a sensor indicates a bump has or will happen.\n");
        return 0;
    }
    
    if (argc == 3)
    {
        stream = fopen(argv[2], "r");
        if (stream == NULL) 
        {
            fprintf(stderr, "File could not be opened!\n");
            return 2;
        }
    }
    else 
    {
        stream = NULL;
    }
    
    wiringPiSetupGpio();
    pinMode (LEFT_MOTOR_FWD_GPIO, OUTPUT);
    pinMode (LEFT_MOTOR_REV_GPIO, OUTPUT);
    pinMode (RIGHT_MOTOR_FWD_GPIO, OUTPUT);
    pinMode (RIGHT_MOTOR_REV_GPIO, OUTPUT);
    
    if (stream)
    {
        while (!feof(stream))
        {
            if (fscanf(stream, "%s", data) > 0)
            {
                execute_motion(data);
            }
        }
        fclose(stream);
    }
    else
    {
        execute_motion(argv[1]);
    }
    
    execute_motion(MOTORS_OFF);
    
    return 0;

} // main


void execute_motion(char *data)
{
    char left_motion = data[0];
    char right_motion = data[1];
    int duration = 0;

    if (sscanf( (data + 2), "%d", &duration ) > 0)
    {
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

        if (duration > 0)
        {
            delay(duration);
        }
    }
}

