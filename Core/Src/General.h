
#ifndef GENERAL_H
#define GENERAL_H

#include "main.h"
#include "TempSensor.h"
#include "DistanceSensor.h"
#include "LightSensor.h"
#include "Accelerometer.h"
#include "GPS.h"
#include <stdio.h>   // Required for sprintf
#include <string.h>  // Required for strlen
#include <math.h>

typedef enum {
	MIDDLE, UP, DOWN, LEFT, RIGHT
} ButtonIndex;

char sign(float value);
void WholeFraction(float value, uint8_t precision, uint32_t *whole,
		uint32_t *decimal);
char YesNo(uint8_t value);

void handleCommand();
void defaultSetup();
void displayTemp(float temp);
void displayDistance(float distance);
void displayGPS(float gps_lat, float gps_long);
void displayAlarmConditions();
void displayLight(uint32_t light);
void handleButton(ButtonIndex btn);
void displayAccel(float x, float y, float z);
void displayDate();




#endif
