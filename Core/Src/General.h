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
	MIDDLE, UP, DOWN, LEFT, RIGHT, USER
} ButtonIndex;

typedef enum {
	D2, D3, D4, D5
} LEDIndex;

char sign(float value);
void WholeFraction(float value, uint8_t precision, uint32_t *whole,
		uint32_t *decimal);
char YesNo(uint8_t value);


void defaultSetup();

void handleCommand();
void displayTemp(char *dest);
void displayDistance(char *dest);
void displayGPS(char *dest);
void displayAccel(char *dest);
void displayDate(char *dest);
void displayAlarmConditions(char *dest);
void displayLight(char *dest);

void disableAlarms();
void checkAlarms();
void enableAlarms();

void handleButton(ButtonIndex btn);
void flashLED(LEDIndex led);
void toggleLED(LEDIndex led);

#endif
