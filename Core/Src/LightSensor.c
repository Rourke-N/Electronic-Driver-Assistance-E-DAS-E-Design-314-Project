#include "LightSensor.h"
#include "General.h"

uint32_t current_light = 0;
uint8_t low_light = 0;

uint32_t getLight() {
	return current_light;
}
uint8_t getLowLight() {
	return low_light;
}
