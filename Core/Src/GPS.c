#include "GPS.h"
#include "General.h"

float gps_lat = 0;
float gps_long = 0;

uint32_t heading = 0;
float speed = 0;

char str_lat[12];
char str_long[12];

char str_head[12];
char str_speed[12];

uint8_t GPS_OK = 0;

uint8_t getGPS_OK(){
	return GPS_OK;
}

float getLat() {
	return gps_lat;
}
float getLong() {
	return gps_long;
}

void update_str() {

	uint32_t lat_whole;
	uint32_t lat_decimal;
	char lat_sign = sign(gps_lat);

	uint32_t long_whole;
	uint32_t long_decimal;
	char long_sign = sign(gps_long);

	WholeFraction(gps_lat, 5, &lat_whole, &lat_decimal);
	WholeFraction(gps_long, 5, &long_whole, &long_decimal);

	sprintf(str_lat, "%c%02lu.%06lu", lat_sign, lat_whole, lat_decimal);
	sprintf(str_long, "%c%03lu.%06lu", long_sign, long_whole, long_decimal);

	sprintf(str_head, "%03lu deg", heading);

	uint32_t speed_whole;
	uint32_t speed_decimal;

	WholeFraction(gps_lat, 1, &speed_whole, &speed_decimal);
	sprintf(str_speed, "%03lu.%01lu", speed_whole, speed_decimal);


}

void str_SPEED_HEAD_OLED(char *dest1, char *dest2) {

	update_str();

	sprintf(dest1, "Heading:   %s\n", str_head);

	sprintf(dest2, "Speed:       %s\n", str_speed);
}

void str_LAT_LONG_OLED(char *dest1, char *dest2) {

	update_str();

	sprintf(dest1, "Lat:    %s\n", str_lat);

	sprintf(dest2, "Long:  %s\n", str_long);
}

void str_GPS_UART(char *dest) {

	update_str();

	sprintf(dest + strlen(dest), "GPSLat:   %s\n", str_lat);

	sprintf(dest + strlen(dest), "GPSLong: %s\n", str_long);
}

