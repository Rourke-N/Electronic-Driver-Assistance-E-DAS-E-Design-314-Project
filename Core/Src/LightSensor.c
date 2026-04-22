#include "LightSensor.h"
#include "General.h"

uint32_t current_light = 0;
uint8_t low_light = 0;

char str_light[10];

void update_str_light(char *dest) {

	sprintf(dest, "%04lu lux", current_light);
}

void str_LUX_UART(char *dest) {

	update_str_light(str_light);

	sprintf(dest + strlen(dest), "Light:      %s\n", str_light);
	//HAL_UART_Transmit(&huart2, (uint8_t*) g_tx_buffer, MESSAGE_LENGTH, 100); //22 to include newline
}

void str_LUX_OLED(char *dest) {
	update_str_light(str_light);
	snprintf(dest,19, "Light:    %s\n", str_light);
}

uint32_t getLight() {
	return current_light;
}
uint8_t getLightWarning() {
	return low_light;
}

void clearLightWarning(uint8_t delay) {

}
