#ifndef LIGHTSENSOR_H
#define LIGHTSENSOR_H
#include "main.h"

uint32_t getLight();
uint8_t getLightWarning();

void clearLightWarning();

void str_LUX_UART(char *dest, size_t size);
void str_LUX_OLED(char *dest, size_t size);

void init_Light_Sensor();

void str_Light_SD(char *dest, size_t size);


#endif
