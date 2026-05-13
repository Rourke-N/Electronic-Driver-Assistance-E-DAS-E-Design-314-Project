#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

#include "main.h"

float getX();
float getY();
float getZ();
uint8_t getUnsafeDriving();
uint8_t getImpactWarning();

uint8_t getMPU_OK();

uint8_t checkAccelStatus(void);

void clearUnsafeWarning(uint8_t delay);
void clearImpactWarning(uint8_t delay);

void MPU6050_Init_1();
void MPU6050_Init_2_A();
void readAccel();

void clearIntFlag();

void updateRawBuffer(int16_t rawX, int16_t rawY, int16_t rawZ);

void getAverageRaw(int16_t *avgX, int16_t *avgY, int16_t *avgZ);

void str_Accel_UART(char *dest, size_t size);

void str_Accel_OLED(char *dest, size_t size);

void str_Accel_SD(char *dest, size_t size);

#endif
