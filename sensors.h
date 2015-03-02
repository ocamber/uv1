/**
* sensors.h - Raspberry Pi UV1 Sensor data 
* 
* Oren Camber 2015-02-22
*
*/

#include <stdbool.h>

#define SENSOR_FILE         "/dev/shm/sensor_data"
#define SHARED_MEMORY_KEY   14721
#define RANGE_INDICATOR         'R'
#define OBSTACLE_INDICATOR      'O'
#define OBSTACLE_LEFT_INDEX     0
#define OBSTACLE_FWD_INDEX      1
#define OBSTACLE_RIGHT_INDEX    2
#define OBSTACLE_BACK_INDEX     3
#define SOUND_INDICATOR         'S'
#define NO_RANGE_INDICATOR      'r'
#define NO_OBSTACLE_INDICATOR   'o'
#define NO_SOUND_INDICATOR      's'
#define POSITIVE_VAL            '+'
#define NEGATIVE_VAL            '-'
#define SENSOR_DATA_END_MARK    '\n'

typedef struct {
    char range;
    char range_val[3];
    char obstacle;
    char obstacle_val[4];
    char sound;
    char sound_val;
    char end_mark;
} SENSOR_DATA;

int access_sensor_memory(SENSOR_DATA**, int);
void release_sensor_memory(int, SENSOR_DATA*);
size_t read_sensor_file(SENSOR_DATA *);
size_t write_sensor_file(SENSOR_DATA *);
void clear_sensor_values(SENSOR_DATA *);
void reset_range_value(SENSOR_DATA *);
void reset_obstacle_value(SENSOR_DATA *);
void reset_sound_value(SENSOR_DATA *);
