#ifndef SD_H
#define SD_H
#include "Keypad.h"
#include "General.h"
#include "myRTC.h"
#include "main.h"
#include "ff.h"

uint8_t getSD_OK();

uint8_t getLogging();
void setLogging(uint8_t log);

float getFuel();
void setFuel(float newfuel);

void setDistance_ODO(float newdistance);
float getDistance_ODO();

void str_dist_ODO_OLED(char *dest, size_t size);

void str_FuelEfficiency_UART(char *dest, size_t size);

void str_FuelEfficiency_OLED(char *dest1, char *dest2, size_t size);

void str_fuel_OLED(char *dest, size_t size);


void myprintf(const char *fmt, ...);
void SD_test(void);

void SD_Log_Data(void);
FRESULT SD_Clear_Log();
FRESULT SD_Delete_Log(void);

void SD_Dump(void);

#endif
