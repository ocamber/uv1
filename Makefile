all : sensord reset_sensors lights motors	# Build everything

sensord : sensord.c sensors.o	# Sensor Daemon
	gcc -lwiringPi -lrt sensors.o sensord.c -o sensord

reset_sensors : reset_sensors.c sensors.o 	# App to clear individual sensor values
	gcc -lwiringPi reset_sensors.c sensors.o -o reset_sensors

lights : lights.c gpio_pins.h		# App to turn forwrd lights on or off
	gcc -lwiringPi lights.c -o lights

motors : motors.c gpio_pins.h		# App to run the motors
	gcc -lwiringPi motors.c -o motors

sensors.o : sensors.c sensors.h gpio_pins.h 	# Sensor support functions
	gcc -lwiringPi -c sensors.c -o sensors.o

clean : 
	rm lights motors reset_sensors sensord sensors.o
	
