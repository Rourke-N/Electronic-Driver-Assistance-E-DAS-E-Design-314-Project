#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

#include "main.h"


float getX();
float getY();
float getZ();
uint8_t getUnsafeDriving();
uint8_t getImpact();


void str_Accel_UART(char *dest);
void str_Accel_OLED(char *dest);

#endif
