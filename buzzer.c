/**
* buzzer.c - Control uv1 forward laser
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
    int ok = 0;
        
    wiringPiSetupGpio();
    pinMode (BUZZER_GPIO, OUTPUT);
    digitalWrite (BUZZER_GPIO, LOW);
    pinMode (BUZZER_GPIO, PWM_OUTPUT);
    
    if (argc == 2)
    {
        if (strcmp("off", argv[1]) == 0) 
        {
            ok = 1;
        }
        else
        {
            int freq = atoi(argv[1]);
            if (freq > 0 && freq < 1024) 
            {
                ok = 1;
                int i;
                for (i=0; i<freq; i++) 
                {
                    pwmWrite(BUZZER_GPIO, i);
                    delay(1);
                }
                for (i=freq; i>=0; i--) 
                {
                    pwmWrite(BUZZER_GPIO, i);
                    delay(1);
                }
            }
        }
    }
    if (!ok)
    {
        printf("Usage: buzzer [1-1023] -or- buzzer off\n");
    }
    return 0;

} // main


