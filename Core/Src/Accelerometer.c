#include "Accelerometer.h"
#include "General.h"

float x_accel = 0;
float y_accel = 0;
float z_accel = 0;

uint8_t MPU_OK = 0;

uint8_t getMPU_OK() {
	return MPU_OK;
}

float getX() {
	return x_accel;
}
float getY() {
	return y_accel;
}
float getZ() {
	return z_accel;
}
uint8_t getUnsafeDriving() {
	return 0;
}

uint8_t getImpactWarning(){
	return 0;
}

void clearUnsafeWarning(uint8_t delay) {

}
void clearImpactWarning(uint8_t delay) {

}

void str_Accel_OLED(char *dest) {

	float magnitude = sqrtf(
			x_accel * x_accel + y_accel * y_accel + z_accel * z_accel);

	uint32_t whole;
	uint32_t decimal;

	WholeFraction(magnitude, 2, &whole, &decimal);

	sprintf(dest, "Accel:    |%lu.%02lu| g\n", whole, decimal);
}

void str_Accel_UART(char *dest) {

	float x = x_accel;
	float y = y_accel;
	float z = z_accel;

	char x_sign = sign(x);
	char y_sign = sign(y);
	char z_sign = sign(z);

	uint32_t x_whole, y_whole, z_whole;
	uint32_t x_decimal, y_decimal, z_decimal;

	WholeFraction(x, 2, &x_whole, &x_decimal);
	WholeFraction(y, 2, &y_whole, &y_decimal);
	WholeFraction(z, 2, &z_whole, &z_decimal);

	sprintf(dest + strlen(dest), "X accel:     %c%lu.%02lu g\n", x_sign,
			x_whole, x_decimal);
	//HAL_UART_Transmit(&huart2, (uint8_t*) g_tx_buffer, MESSAGE_LENGTH, 100);

	sprintf(dest + strlen(dest), "Y accel:     %c%lu.%02lu g\n", y_sign,
			y_whole, y_decimal);
	//HAL_UART_Transmit(&huart2, (uint8_t*) g_tx_buffer, MESSAGE_LENGTH, 100);

	sprintf(dest + strlen(dest), "X accel:     %c%lu.%02lu g\n", z_sign,
			z_whole, z_decimal);
	//HAL_UART_Transmit(&huart2, (uint8_t*) g_tx_buffer, MESSAGE_LENGTH, 100);
}

