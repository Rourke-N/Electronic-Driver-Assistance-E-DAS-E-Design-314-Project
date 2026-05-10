#ifndef LIGHTSENSOR_H
#define LIGHTSENSOR_H
#include "main.h"

uint32_t getLight();
uint8_t getLightWarning();

void clearLightWarning(uint8_t delay);

void str_LUX_UART(char *dest, size_t size);
void str_LUX_OLED(char *dest, size_t size);

void init_Light_Sensor();

#endif
