#include <myGPS.h>
#include "General.h"

float gps_lat = 0;
float gps_long = 0;

float heading = 0;
float speed = 0;

char str_lat[15];
char str_long[15];

char str_head[15];
char str_speed[15];

char GPS_Buffer[100];

#define GPS_Buffer_Size 100

uint8_t GPS_OK = 0;

uint8_t getGPS_OK() {
	return GPS_OK;
}



float getSpeed() {
	speed = getSpeed_gps();
	return speed;
}

float getHeading() {
	heading = getHeading_gps();
	return heading;
}

float getLat() {
	gps_lat = getLat_gps();

	return gps_lat;
}
float getLong() {

	gps_long = getLong_gps();
	return gps_long;
}
void update_str() {
    getLat();
    getLong();
    getHeading();
    getSpeed();

    uint32_t lat_whole;
    uint32_t lat_decimal;
    char lat_sign = sign(gps_lat);

    uint32_t long_whole;
    uint32_t long_decimal;
    char long_sign = sign(gps_long);

    WholeFraction(gps_lat, 5, &lat_whole, &lat_decimal);
    WholeFraction(gps_long, 5, &long_whole, &long_decimal);

    // Using sizeof() here because these arrays are defined in this file scope
    snprintf(str_lat, sizeof(str_lat), "%c%02lu.%06lu", lat_sign, lat_whole, lat_decimal);
    snprintf(str_long, sizeof(str_long), "%c%03lu.%06lu", long_sign, long_whole, long_decimal);

    snprintf(str_head, sizeof(str_head), "%03d", (int)heading);

    uint32_t speed_whole;
    uint32_t speed_decimal;

    WholeFraction(speed, 1, &speed_whole, &speed_decimal);
    snprintf(str_speed, sizeof(str_speed), "%03lu.%01lu", speed_whole, speed_decimal);
}

void str_SPEED_HEAD_OLED(char *dest1, char *dest2, size_t size) {
    update_str();

    // Exactly matches your spacing and format
    snprintf(dest1, size, "Heading:   %s deg\n", str_head);
    snprintf(dest2, size, "Speed:  %s km/h\n", str_speed);
}

void str_LAT_LONG_OLED(char *dest1, char *dest2, size_t size) {
    update_str();

    // Exactly matches your spacing and format
    snprintf(dest1, size, "Lat:    %s\n", str_lat);
    snprintf(dest2, size, "Long:  %s\n", str_long);
}

void str_GPS_UART(char *dest, size_t size) {
    update_str();

    size_t len = strlen(dest);
    if (len < size) {
        // Appends GPSLat, then updates len to append GPSLong
        len += snprintf(dest + len, size - len, "GPSLat:   %s\n", str_lat);

        if (len < size) {
            snprintf(dest + len, size - len, "GPSLong: %s\n", str_long);
        }
    }
}
