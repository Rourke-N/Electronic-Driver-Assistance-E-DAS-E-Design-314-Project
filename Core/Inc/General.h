#ifndef GENERAL_H
#define GENERAL_H
#include "main.h"

#include "OLED.h"
#include "SD.h"
#include "TempSensor.h"
#include "DistanceSensor.h"
#include "LightSensor.h"
#include "Accelerometer.h"
#include "GPS.h"
#include "Keypad.h"
#include <stdio.h>   // Required for sprintf
#include <string.h>  // Required for strlen
#include <math.h>
#include "OLED.h"

#define NUM_BUTTONS 6

char sign(float value);
void WholeFraction(float value, uint8_t precision, uint32_t *whole,
		uint32_t *decimal);
char YesNo(uint8_t value);

void scanButtons();
typedef enum {
	MIDDLE, UP, DOWN, LEFT, RIGHT, USER
} ButtonIndex;


void defaultSetup();
void mainLoop();
void handleCommand();
void str_GPS_UART(char *dest);
void str_Date_UART(char *dest, uint8_t space);
void str_AlarmConditions_UART(char *dest);

void disableAlarmChecks();
void checkAlarms();
void enableAlarms();

//LEDS
typedef enum {
	D2, D3, D4, D5
} LEDIndex;



#define LED_FLASHING 1000
#define LED_OFF 0
#define LED_ON 2000

void handleButton(ButtonIndex btn);
void flashLED(LEDIndex led);
void toggleLED(LEDIndex led);

//OLED
void UI_Refresh();
void UI_handleKey(char key);

#endif
