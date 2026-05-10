#include "LightSensor.h"
#include "General.h"

uint32_t current_light = 0;
uint8_t light_warning = 0;

char str_light[10];

extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;

volatile uint16_t SensorBuffer[10] = { 0 };

const float conversion_factor = 0.2197; //4096*conv = 900

void init_Light_Sensor() {

	//HAL_ADCEx_Calibration_Start(&hadc1);

	//HAL_ADC_Start(&hadc1);
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*) SensorBuffer, 10);

}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
	if (hadc->Instance == ADC1) {
		uint32_t sum = 0;
		for (int i = 0; i < 10; i++) {
			sum += SensorBuffer[i];
		}
		current_light = (sum / 10) * conversion_factor;
	}
}

void update_str_light(char *dest, size_t size) {
	// Safety: formatting the light value into the provided buffer
	snprintf(dest, size, "%04lu lux", current_light);
}

void str_LUX_UART(char *dest, size_t size) {
	// Update the local str_light buffer first
	update_str_light(str_light, sizeof(str_light));

	// Calculate current length and remaining space
	size_t len = strlen(dest);
	if (len < size) {
		snprintf(dest + len, size - len, "Light:      %s\n", str_light);
	}
}

void str_LUX_OLED(char *dest, size_t size) {
	// Update the local str_light buffer first
	update_str_light(str_light, sizeof(str_light));

	// Safe format into the OLED display buffer
	snprintf(dest, size, "Light:    %s\n", str_light);
}

uint32_t getLight() {
	return current_light;
}
uint8_t getLightWarning() {

	if (current_light < 300) {
		light_warning = 1;
	} else if (current_light > 400) {
		light_warning = 0;
	}

	return light_warning;
}

void clearLightWarning(uint8_t delay) {
	light_warning = 0;
}
