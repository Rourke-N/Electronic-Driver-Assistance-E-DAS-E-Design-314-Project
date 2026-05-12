#include "myRTC.h"

extern RTC_HandleTypeDef hrtc;

// Sets the RTC hardware registers directly using local HAL structs.
// No dependency on main.c's sDate/sTime variables.
void Set_RTC_DateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second) {
    RTC_TimeTypeDef sTimeSet = {0};
    RTC_DateTypeDef sDateSet = {0};

    // 1. Prepare structures using BINARY format (sscanf gives us decimal/binary)
    sDateSet.Year = (uint8_t)(year % 100);
    sDateSet.Month = month;
    sDateSet.Date = day;
    sDateSet.WeekDay = RTC_WEEKDAY_MONDAY; // MANDATORY for many STM32s

    sTimeSet.Hours = hour;
    sTimeSet.Minutes = minute;
    sTimeSet.Seconds = second;

    // 2. Write to Hardware - Order matters on some models (Time then Date)
    if (HAL_RTC_SetTime(&hrtc, &sTimeSet, RTC_FORMAT_BIN) != HAL_OK) {
        // Error handling
    }
    if (HAL_RTC_SetDate(&hrtc, &sDateSet, RTC_FORMAT_BIN) != HAL_OK) {
        // Error handling
    }
}
// For SD card logging — writes "YYYY/MM/DD HH:MM:SS" into dest from position 0.
// dest should be a fresh field buffer (as used in SD_Log_Data).
// The caller (SD_Log_Data) appends the comma separator after this.
void str_Date_SD(char *dest, size_t size) {
    RTC_TimeTypeDef gTime = {0};
    RTC_DateTypeDef gDate = {0};

    // HAL requires reading Time first, then Date to unlock shadow registers
    if (HAL_RTC_GetTime(&hrtc, &gTime, RTC_FORMAT_BIN) == HAL_OK) {
        HAL_RTC_GetDate(&hrtc, &gDate, RTC_FORMAT_BIN);

        // Space separator between date and time — matches PDD Table 9 format
        // Comma is added by SD_Log_Data after this call, not here
        snprintf(dest, size, "%04u/%02u/%02u %02u:%02u:%02u",
                 2000 + gDate.Year, gDate.Month, gDate.Date,
                 gTime.Hours, gTime.Minutes, gTime.Seconds);
    }
}

void str_Date_OLED(char *dest, size_t size) {
    RTC_TimeTypeDef gTime = {0};
    RTC_DateTypeDef gDate = {0};

    // HAL requires reading Time first, then Date to unlock shadow registers
    if (HAL_RTC_GetTime(&hrtc, &gTime, RTC_FORMAT_BIN) == HAL_OK) {
        HAL_RTC_GetDate(&hrtc, &gDate, RTC_FORMAT_BIN);

        // Space separator between date and time — matches PDD Table 9 format
        // Comma is added by SD_Log_Data after this call, not here
        snprintf(dest, size, "=%04u/%02u/%02u %02u:%02u:%02u=",
                 2000 + gDate.Year, gDate.Month, gDate.Date,
                 gTime.Hours, gTime.Minutes, gTime.Seconds);
    }
}

// For UART Stat response — appends "YYYY/MM/DD HH:MM:SS \n" to dest.
// Trailing space before \n pads line to 21 characters per PDD Table 4.
void str_Date_UART(char *dest, size_t size) {
    RTC_TimeTypeDef gTime = {0};
    RTC_DateTypeDef gDate = {0};

    if (HAL_RTC_GetTime(&hrtc, &gTime, RTC_FORMAT_BIN) == HAL_OK) {
        HAL_RTC_GetDate(&hrtc, &gDate, RTC_FORMAT_BIN);

        size_t len = strlen(dest);
        if (len < size) {
            // Trailing space before \n — PDD requires 21 chars per line
            snprintf(dest + len, size - len,
                     "%04u/%02u/%02u %02u:%02u:%02u \n",
                     2000 + gDate.Year, gDate.Month, gDate.Date,
                     gTime.Hours, gTime.Minutes, gTime.Seconds);
        }
    }
}
