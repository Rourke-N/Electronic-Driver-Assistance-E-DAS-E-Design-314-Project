#ifndef LIGHTSENSOR_H
#define LIGHTSENSOR_H
#include "main.h"

uint32_t getLight();
uint8_t getLowLight();

void str_LUX_UART(char *dest);
void str_LUX_OLED(char *dest);

#endif
