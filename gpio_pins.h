/**
* gpio_pins.h - Raspberry Pi UV1 GPIO pin map
* 
* Oren Camber 2014-05-21
*
*/

//      3V Power                    Pin 01  Orange
//      5V Power                        02  Red
//      SDA1 I2C           GPIO  2      03  --
//      5V Power                        04  Red
//      SCL1 I2C                 3      05  --
//      Ground                          06  Brown
#define SOUND_GPIO               4  //  07  White
//      UART0_TXD               14      08  Yellow
//      Ground                          09  Black
//      UART0_RXD               15      10  Green
#define LEFT_MOTOR_FWD_GPIO     17  //  11  Blue
//      PCM_CLK                 18      12  Gray
#define LEFT_MOTOR_REV_GPIO     27  //  13  Green
//      Ground                          14  Black
#define RIGHT_MOTOR_FWD_GPIO    22  //  15  Yellow
#define RIGHT_MOTOR_REV_GPIO    23  //  16  Orange
//      3V Power                        17  Orange
#define LASER_GPIO              24  //  18  Red
//      SPI0_MOSI               10  //  19  --
//      Ground                          20  Black
//      SPI0_MISO                9  //  21  --
#define LIGHTS_GPIO             25  //  22  Yellow
//      SPI0_SCLK               11  //  23  --
//      SPI0_CE0_N               8  //  23  --
//      Ground                          25  --
//      SPI0_CE1_N               7  //  26  --
//      I2C ID_SD                   //  27  --
//      I2C ID_SC                   //  28  --
#define RANGE_ECHO_GPIO          5  //  29  Gray
//      Ground                          30  Black
#define RANGE_TRIGGER_GPIO       6  //  31  Purple
//                              12  //  32  Blue    <-- AVAIL
//                              13  //  33  Green   <-- AVAIL
//      Ground                          34  Black
#define OBSTACLE_B_GPIO         19  //  35  Blue
#define OBSTACLE_F_GPIO         16  //  36  Purple
#define OBSTACLE_L_GPIO         26  //  37  Green
#define OBSTACLE_R_GPIO         20  //  38  Gray
//      Ground                          39  Black
//                              21  //  40  White   <-- AVAIL