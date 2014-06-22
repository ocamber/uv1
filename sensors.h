/**
* sensors.h - Raspberry Pi UV1 Sensor data 
* 
* Oren Camber 2014-05-21
*
*/

#include <stdbool.h>

#define SENSOR_FILE             "/dev/shm/sensor_data"
#define SHARED_MEMORY_KEY       14721
#define TOUCH_INDICATOR         'T'
#define OBSTACLE_INDICATOR      'O'
#define SOUND_INDICATOR         'S'
#define RANGE_INDICATOR         'R'
#define NO_TOUCH_INDICATOR      't'
#define NO_OBSTACLE_INDICATOR   'o'
#define NO_SOUND_INDICATOR      's'
#define ECHO_INDICATOR          'e'
#define NO_RANGE_INDICATOR      'r'
#define POSITIVE_VAL            '+'
#define NEGATIVE_VAL            '-'
#define POSITIVE_LEFT           '<'
#define POSITIVE_RIGHT          '>'
#define SENSOR_DATA_END_MARK    '\n'

typedef struct {
    char touch;
    char touch_val;
    char obstacle;
    char obstacle_val;
    char sound;
    char sound_val;
    char range;
    char range_val[3];
    char end_mark;
} SENSOR_DATA;

void release_shared_memory(SENSOR_DATA*, int);
int access_shared_memory(SENSOR_DATA*);
int access_sensors(SENSOR_DATA*);
int initialize_sensors(SENSOR_DATA *);
size_t read_sensor_file(SENSOR_DATA *);
size_t write_sensor_file(SENSOR_DATA *);
void clear_sensor_values(SENSOR_DATA *);
void reset_touch_value(SENSOR_DATA *);
void reset_obstacle_value(SENSOR_DATA *);
void reset_sound_value(SENSOR_DATA *);
void reset_range_value(SENSOR_DATA *);
