#include "SD.h"
#include "Keypad.h"
#include "General.h"

char str_km_l[12];
char str_l_100km[15];

char str_fuel[12];

float fuel;
float distance;

uint8_t log_data = 0;

uint8_t SD_OK = 0;
const float MAX_EFF = 99.9;

uint8_t getSD_OK() {
	return SD_OK;
}

void setLogging(uint8_t log) {
	log_data = log;
}

uint8_t getLogging() {
	return log_data;
}

void setDistance_ODO(float newdistance) {
	distance = newdistance;
}
float getDistance_ODO() {
	return distance;
}

void setFuel(float newfuel) {
	fuel = newfuel;
}

float getFuel() {
	return fuel;
}

void update_strs() {
	float km_l;
	if (fuel != 0) {
		km_l = distance / fuel;
		if (km_l > MAX_EFF) {
			km_l = MAX_EFF;
		}
	} else {
		km_l = 0;
	}

	float l_100km;
	if (distance != 0) {
		l_100km = 100.0f * (fuel / distance);
		if (l_100km > MAX_EFF) {
			l_100km = MAX_EFF;
		}
	} else {
		l_100km = 0;
	}

	uint32_t km_l_whole;
	uint32_t km_l_decimal;

	uint32_t l_100km_whole;
	uint32_t l_100km_decimal;

	WholeFraction(km_l, 1, &km_l_whole, &km_l_decimal);
	WholeFraction(l_100km, 1, &l_100km_whole, &l_100km_decimal);

	sprintf(str_km_l, "%02lu.%01lu km/L", km_l_whole, km_l_decimal);
	sprintf(str_l_100km, " %02lu.%01lu L/100 km", l_100km_whole,
			l_100km_decimal);

}

void str_fuel_OLED(char *dest) {

	uint32_t fuel_whole;
	uint32_t fuel_decimal;

	WholeFraction(fuel, 1, &fuel_whole, &fuel_decimal);

	sprintf(dest, "Current:   %03lu.%01lu L", fuel_whole, fuel_decimal);

}

void str_dist_ODO_OLED(char *dest) {

	uint32_t dist_whole;
	uint32_t dist_decimal;

	WholeFraction(distance, 1, &dist_whole, &dist_decimal);

	sprintf(dest, "Current:  %03lu.%01lu km", dist_whole, dist_decimal);

}

void str_FuelEfficiency_UART(char *dest) {

	update_strs();

	sprintf(dest + strlen(dest), "Fuel Eff: %s%s\n", str_km_l, str_l_100km);
}

void str_FuelEfficiency_OLED(char *dest1, char *dest2) {

	update_strs();

	sprintf(dest1, "         %s\n", str_km_l);

	sprintf(dest2, "    %s\n", str_l_100km);
}
