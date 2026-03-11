#ifndef TEMPSENSOR_H_
#define TEMPSENSOR_H_

#include "stm32f4xx_hal.h" // Needed for TIM_HandleTypeDef
#include "General.h"
#include "main.h"

void sampleTempSensor();
float getTemp();
uint8_t getTempWarning();

#endif
