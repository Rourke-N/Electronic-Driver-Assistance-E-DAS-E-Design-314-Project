#ifndef GPS_H
#define GPS_H
#include "main.h"

float getLat();
float getLong();
void str_GPS_UART(char *dest);
void str_LAT_LONG_OLED(char *dest1, char *dest2);
void str_SPEED_HEAD_OLED(char *dest1, char *dest2);


#endif
