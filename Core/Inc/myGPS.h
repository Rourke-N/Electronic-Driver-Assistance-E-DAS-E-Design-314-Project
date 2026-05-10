#ifndef GPS_H
#define GPS_H
#include "main.h"

float getLat();
float getLong();

uint8_t getGPS_OK();
void initGPS();


void str_SPEED_HEAD_OLED(char *dest1, char *dest2, size_t size);

void str_LAT_LONG_OLED(char *dest1, char *dest2, size_t size);

void str_GPS_UART(char *dest, size_t size);

#endif
