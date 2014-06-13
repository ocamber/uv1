/**
* lights.c - Control uv1 forward lights
* 
* Oren Camber 2014-05-21
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
        printf("Usage: lights on  -or-  lights off\n");
        return 0;
    }
        
    wiringPiSetupGpio();
    pinMode (LIGHTS_GPIO, OUTPUT);
    
    if (strcmp("on", argv[1]) == 0) 
    {
        digitalWrite(LIGHTS_GPIO, HIGH);
    }
    else
    {
        digitalWrite(LIGHTS_GPIO, LOW);
    }
    return 0;

} // main


