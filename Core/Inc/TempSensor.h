#ifndef TEMPSENSOR_H_
#define TEMPSENSOR_H_
#include "General.h"

void sampleTempSensor();
float getTemp();

uint8_t getTempWarning();
void enableTempAlarmCheck();
void disableTempAlarmCheck();
void clearTempWarning(uint8_t delay);


void str_temp_OLED(char *dest, size_t size);

void str_temp_UART(char *dest, size_t size);

#endif
