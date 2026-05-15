#ifndef MYRTC_H
#define MYRTC_H
#include "main.h"
#include "General.h"

void Set_RTC_DateTime(uint16_t year, uint8_t month, uint8_t day,
                      uint8_t hour,  uint8_t minute, uint8_t second);
void str_Date_UART(char *dest, size_t size);
void str_Date_SD(char *dest, size_t size);

void str_Date_OLED(char *dest, size_t size);

void initRTC();
#endif
