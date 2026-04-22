#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

#include "main.h"


float getX();
float getY();
float getZ();
uint8_t getUnsafeDriving();
uint8_t getImpactWarning();

uint8_t getMPU_OK();

void clearUnsafeWarning(uint8_t delay);
void clearImpactWarning(uint8_t delay);


void str_Accel_UART(char *dest);
void str_Accel_OLED(char *dest);

#endif
