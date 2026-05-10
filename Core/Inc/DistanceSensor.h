#ifndef DISTANCESENSOR_H
#define DISTANCESENSOR_H

#include "main.h"

void sampleDistanceSensor();
float getDistance();

uint8_t getProximityWarning();
void clearProximityWarning(uint8_t delay);

void enableDistanceAlarmCheck();
void disableDistanceAlarmCheck();
void set_DistanceAlarm(uint8_t on);

void str_dist_UART(char *dest, size_t size);

void str_dist_OLED(char *dest, size_t size);

void distance_alarm_condition();

#endif
