/*
 * gps.c
 *
 *  Created on: Nov 15, 2019
 *      Author: Bulanov Konstantin
 *
 *  Contact information
 *  -------------------
 *
 * e-mail   :  leech001@gmail.com
 */

/*
 * |---------------------------------------------------------------------------------
 * | Copyright (C) Bulanov Konstantin,2019
 * |
 * | This program is free software: you can redistribute it and/or modify
 * | it under the terms of the GNU General Public License as published by
 * | the Free Software Foundation, either version 3 of the License, or
 * | any later version.
 * |
 * | This program is distributed in the hope that it will be useful,
 * | but WITHOUT ANY WARRANTY; without even the implied warranty of
 * | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * | GNU General Public License for more details.
 * |
 * | You should have received a copy of the GNU General Public License
 * | along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * |---------------------------------------------------------------------------------
 */

#include <stdio.h>
#include <string.h>
//#include <usart.h>
#include <main.h>
#include "gps.h"

#if (GPS_DEBUG == 1)
#include <usbd_cdc_if.h>
#endif

extern volatile uint8_t rx_data;
uint8_t rx_buffer[GPSBUFSIZE];
uint8_t rx_index = 0;
// In gps.c
volatile uint8_t GPS_data_ready = 0;

GPS_t GPS;

#if (GPS_DEBUG == 1)
void GPS_print(char *data){
	char buf[GPSBUFSIZE] = {0,};
	sprintf(buf, "%s\n", data);
	CDC_Transmit_FS((unsigned char *) buf, (uint16_t) strlen(buf));
}
#endif

void GPS_Init()
{
	HAL_UART_Receive_IT(GPS_USART, &rx_data, 1);
}


void GPS_UART_CallBack(){
	if (rx_data != '\n' && rx_index < sizeof(rx_buffer)) {
		rx_buffer[rx_index++] = rx_data;
	} else {

		#if (GPS_DEBUG == 1)
		GPS_print((char*)rx_buffer);
		#endif

		if(GPS_validate((char*) rx_buffer))
			GPS_parse((char*) rx_buffer);
		rx_index = 0;
		memset(rx_buffer, 0, sizeof(rx_buffer));
	}
	//HAL_UART_Receive_IT(GPS_USART, (uint8_t*)&rx_data, 1);
}


int GPS_validate(char *nmeastr){
    char check[3];
    char checkcalcstr[3];
    int i;
    int calculated_check;

    i=0;
    calculated_check=0;

    // check to ensure that the string starts with a $
    if(nmeastr[i] == '$')
        i++;
    else
        return 0;

    //No NULL reached, 75 char largest possible NMEA message, no '*' reached
    while((nmeastr[i] != 0) && (nmeastr[i] != '*') && (i < 75)){
        calculated_check ^= nmeastr[i];// calculate the checksum
        i++;
    }

    if(i >= 75){
        return 0;// the string was too long so return an error
    }

    if (nmeastr[i] == '*'){
        check[0] = nmeastr[i+1];    //put hex chars in check string
        check[1] = nmeastr[i+2];
        check[2] = 0;
    }
    else
        return 0;// no checksum separator found there for invalid

    sprintf(checkcalcstr,"%02X",calculated_check);
    return((checkcalcstr[0] == check[0])
        && (checkcalcstr[1] == check[1])) ? 1 : 0 ;
}

// In gps.c
void GPS_parse(char *GPSstrParse){
    // Skip the first character '$' and the next two (GP, GN, BD, etc.)
    // Look at index 3 for the sentence type (GGA, RMC, etc.)

    if(!strncmp(GPSstrParse + 3, "GGA", 3)){
        // Use %*5s to skip the "$GNGGA" header entirely
        if (sscanf(GPSstrParse, "$%*5s,%f,%f,%c,%f,%c,%d,%d,%f,%f,%c",
            &GPS.utc_time, &GPS.nmea_latitude, &GPS.ns, &GPS.nmea_longitude,
            &GPS.ew, &GPS.lock, &GPS.satelites, &GPS.hdop, &GPS.msl_altitude, &GPS.msl_units) >= 1){
            GPS.dec_latitude = GPS_nmea_to_dec(GPS.nmea_latitude, GPS.ns);
            GPS.dec_longitude = GPS_nmea_to_dec(GPS.nmea_longitude, GPS.ew);
            return;
        }
    }
    else if (!strncmp(GPSstrParse + 3, "RMC", 3)){
        char status; // NEW: To handle the 'A' or 'V' field
        // Added %c after the first %f to catch the 'A' status
        if(sscanf(GPSstrParse, "$%*5s,%f,%c,%f,%c,%f,%c,%f,%f,%d",
            &GPS.utc_time, &status, &GPS.nmea_latitude, &GPS.ns, &GPS.nmea_longitude,
            &GPS.ew, &GPS.speed_k, &GPS.course_d, &GPS.date) >= 1) {

            // Map the speed from knots to km/h for your OLED
            GPS.speed_km = GPS.speed_k * 1.852f;
            GPS.dec_latitude = GPS_nmea_to_dec(GPS.nmea_latitude, GPS.ns);
            GPS.dec_longitude = GPS_nmea_to_dec(GPS.nmea_longitude, GPS.ew);
            return;
        }
    }
}

/*
void GPS_parse(char *GPSstrParse){
    if(!strncmp(GPSstrParse, "$GPGGA", 6)){
    	if (sscanf(GPSstrParse, "$GPGGA,%f,%f,%c,%f,%c,%d,%d,%f,%f,%c", &GPS.utc_time, &GPS.nmea_latitude, &GPS.ns, &GPS.nmea_longitude, &GPS.ew, &GPS.lock, &GPS.satelites, &GPS.hdop, &GPS.msl_altitude, &GPS.msl_units) >= 1){
    		GPS.dec_latitude = GPS_nmea_to_dec(GPS.nmea_latitude, GPS.ns);
    		GPS.dec_longitude = GPS_nmea_to_dec(GPS.nmea_longitude, GPS.ew);
    		return;
    	}
    }
    else if (!strncmp(GPSstrParse, "$GPRMC", 6)){
    	if(sscanf(GPSstrParse, "$GPRMC,%f,%f,%c,%f,%c,%f,%f,%d", &GPS.utc_time, &GPS.nmea_latitude, &GPS.ns, &GPS.nmea_longitude, &GPS.ew, &GPS.speed_k, &GPS.course_d, &GPS.date) >= 1)
    		return;

    }
    else if (!strncmp(GPSstrParse, "$GPGLL", 6)){
        if(sscanf(GPSstrParse, "$GPGLL,%f,%c,%f,%c,%f,%c", &GPS.nmea_latitude, &GPS.ns, &GPS.nmea_longitude, &GPS.ew, &GPS.utc_time, &GPS.gll_status) >= 1)
            return;
    }
    else if (!strncmp(GPSstrParse, "$GPVTG", 6)){
        if(sscanf(GPSstrParse, "$GPVTG,%f,%c,%f,%c,%f,%c,%f,%c", &GPS.course_t, &GPS.course_t_unit, &GPS.course_m, &GPS.course_m_unit, &GPS.speed_k, &GPS.speed_k_unit, &GPS.speed_km, &GPS.speed_km_unit) >= 1)
            return;
    }
}
*/
float GPS_nmea_to_dec(float deg_coord, char nsew) {
    int degree = (int)(deg_coord/100);
    float minutes = deg_coord - degree*100;
    float dec_deg = minutes / 60;
    float decimal = degree + dec_deg;
    if (nsew == 'S' || nsew == 'W') { // return negative
        decimal *= -1;
    }
    return decimal;
}

//I have added;

float getSpeed_gps(){
	return GPS.speed_km;
}

float getHeading_gps(){
	return GPS.course_d;
}

float getLat_gps(){
	return GPS.dec_latitude;
}

float getLong_gps(){
	return GPS.dec_longitude;
}
