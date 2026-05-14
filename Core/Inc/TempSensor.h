#ifndef TEMPSENSOR_H_
#define TEMPSENSOR_H_
#include "General.h"

void sampleTempSensor();
float getTemp();

uint8_t getTempWarning();
void enableTempAlarmCheck();
void disableTempAlarmCheck();
void clearTempWarning();


void str_temp_OLED(char *dest, size_t size);

void str_temp_UART(char *dest, size_t size);

// Format: ±xx.x  (sign is '-' or nothing, one decimal place)
void str_Temp_SD(char *dest, size_t size);


#endif
