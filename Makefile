all : sensord reset_sensors lights laser buzzer motors	# Build everything

sensord : sensord.c sensors.o	# Sensor Daemon
	gcc -lwiringPi -lrt sensors.o sensord.c -o sensord

reset_sensors : reset_sensors.c sensors.o 	# App to clear individual sensor values
	gcc -lwiringPi reset_sensors.c sensors.o -o reset_sensors

lights : lights.c gpio_pins.h		# App to turn forwrd lights on or off
	gcc -lwiringPi lights.c -o lights

laser : laser.c gpio_pins.h		# App to turn forward laser on or off
	gcc -lwiringPi laser.c -o laser

buzzer : buzzer.c gpio_pins.h		# App to turn buzzer on or off
	gcc -lwiringPi buzzer.c -o buzzer

motors : motors.c gpio_pins.h		# App to run the motors
	gcc -lwiringPi motors.c -o motors

sensors.o : sensors.c sensors.h gpio_pins.h 	# Sensor support functions
	gcc -lwiringPi -c sensors.c -o sensors.o

clean : 
	rm lights laser buzzer motors reset_sensors sensord sensors.o
	
