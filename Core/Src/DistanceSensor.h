#ifndef DISTANCESENSOR_H
#define DISTANCESENSOR_H

#include "main.h"

void sampleDistanceSensor();
float getDistance();
uint8_t getImpact();
uint8_t getProximityWarning();
void enableDistanceAlarmCheck();
void disableDistanceAlarmCheck();

#endif
