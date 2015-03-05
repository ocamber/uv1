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
#define SOUND_INDICATOR         'S'
#define IMPACT_INDICATOR        'I'
#define NO_RANGE_INDICATOR      'r'
#define NO_OBSTACLE_INDICATOR   'o'
#define NO_SOUND_INDICATOR      's'
#define NO_IMPACT_INDICATOR     'i'
#define POSITIVE_VAL            '+'
#define NEGATIVE_VAL            '-'
#define IDX_FWD                 0
#define IDX_BACK                1
#define IDX_LEFT                2
#define IDX_RIGHT               3
#define SENSOR_DATA_END_MARK    '\n'

typedef struct {
    char range_indic;       // 'R'/'r'
    char range_val[3];      // 000-999
    char obstacle_indic;    // 'O'/'o'
    char obstacle_val[4];   // [ F B L R ] +/-
    char sound_indic;       // 'S'/'s'
    char sound_val;         // +/-
    char impact_indic;      // 'I'/'i'
    char impact_val[2];     // [ F B ] +/-
    char end_mark;          
} SENSOR_DATA;

int access_sensor_memory(SENSOR_DATA**, int);
void release_sensor_memory(int, SENSOR_DATA*);
size_t read_sensor_file(SENSOR_DATA *);
size_t write_sensor_file(SENSOR_DATA *);
void clear_sensor_values(SENSOR_DATA *);
void reset_range(SENSOR_DATA *);
void reset_obstacle(SENSOR_DATA *);
void reset_sound(SENSOR_DATA *);
void reset_impact(SENSOR_DATA *);
