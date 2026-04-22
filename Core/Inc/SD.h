#ifndef SD_H
#define SD_H
#include "main.h"

uint8_t getSD_OK();

uint8_t getLogging();
void setLogging(uint8_t log);

float getFuel();
void setFuel(float newfuel);

void setDistance_ODO(float newdistance);
float getDistance_ODO();

void str_FuelEfficiency_OLED(char *dest1, char *dest2);
void str_fuel_OLED(char *dest);

void str_dist_ODO_OLED(char *dest);
void str_FuelEfficiency_UART(char *dest1);

#endif
