/**
* laser.c - Control uv1 forward laser
* 
* Oren Camber 2014-06-21
*
* compile with -lwiringPi
*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include "gpio_pins.h"

int main(int argc, char **argv)
{
    
    if (argc != 2 || (argc==2 && strcmp("on", argv[1]) != 0 && strcmp("off", argv[1]) != 0))
    {
        printf("Usage: laser on  -or-  laser off\n");
        return 0;
    }
        
    wiringPiSetupGpio();
    pinMode (LASER_GPIO, OUTPUT);
    
    if (strcmp("on", argv[1]) == 0) 
    {
        digitalWrite(LASER_GPIO, HIGH);
    }
    else
    {
        digitalWrite(LASER_GPIO, LOW);
    }
    return 0;

} // main


