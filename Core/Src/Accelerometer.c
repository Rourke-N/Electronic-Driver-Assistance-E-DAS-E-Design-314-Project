#include "Accelerometer.h"

float x_accel = 0;
float y_accel = 0;
float z_accel = 0;

float getX(){
	return x_accel;
}
float getY(){
	return y_accel;
}
float getZ(){
	return z_accel;
}
uint8_t getUnsafeDriving(){
	return 0;
}

uint8_t getImpact(){
	return 0;
}
