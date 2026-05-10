#include <myGPS.h>
#include "OLED.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"
#include <stdio.h>
#include "TempSensor.h"
#include "DistanceSensor.h"
#include "LightSensor.h"
#include "SD.h"
//1: Display
//2: Data
//3: Diagnostics

MenuElement_t Display_main, Display_1, Display_2, Display_3, Display_4,
		Display_5;
MenuElement_t Data_main, Data_1, Data_2, Data_3;
MenuElement_t Diagnostics_main, Diagnostics_1, Diagnostics_2, Diagnostics_3,
		Diagnostics_4;

MenuElement_t Warn_UnsafeDriving, Warn_Proximity, Warn_Light, Warn_Temp,
		Warn_Impact;

const char *STR_MEAS_HEADER = "== Measurements ==";
const char *STR_DATA_HEADER = "=== Data Entry ===";
const char *STR_DIAG_HEADER = "== Diagnostics ===";
const char *STR_DIAG_SUB_HEADER = "-- Diagnostics ---";
const char *STR_WARN_HEADER = "===== WARNING ====";
const char *STR_PRESS_DISPLAY = "Press -> display";
const char *STR_S3_CHANGE = "Press S3 to change";
const char *STR_S3_ACCEPT = "Press S3 to accept";

const char *STR_SD_OK = "SD-card:        OK";
const char *STR_SD_FAIL = "SD-card:    NOT OK";

const char *STR_GPS_OK = "GPS:            OK";
const char *STR_GPS_FAIL = "GPS:        NOT OK";

const char *STR_MPU_OK = "MPU-6050:       OK";
const char *STR_MPU_FAIL = "MPU-6050:   NOT OK";

const char *STR_LOG_ENABLED = "Log Data:  ENABLED";
const char *STR_LOG_DISABLED = "Log Data: DISABLED";


char S3_STATE[18];
char LOG_STATE[18];

char SD_DIAG[18];
char GPS_DIAG[18];
char MPU_DIAG[18];

char row[3][30];

#define ROW_SIZE 30


void str_toggleS3(uint8_t editing) {
	if (editing) {
		strcpy(S3_STATE, STR_S3_ACCEPT);
	} else {
		strcpy(S3_STATE, STR_S3_CHANGE);
	}
}



void str_toggleSD_State(uint8_t ok) {
	if (ok) {
		strcpy(SD_DIAG, STR_SD_OK);
	} else {
		strcpy(SD_DIAG, STR_SD_FAIL);
	}
}

void str_toggleGPS_State(uint8_t ok) {
	if (ok) {
		strcpy(GPS_DIAG, STR_GPS_OK);
	} else {
		strcpy(GPS_DIAG, STR_GPS_FAIL);
	}
}

void str_toggleMPU_State(uint8_t ok) {
	if (ok) {
		strcpy(MPU_DIAG, STR_MPU_OK);
	} else {
		strcpy(MPU_DIAG, STR_MPU_FAIL);
	}
}



void str_toggleLOG() {

	uint8_t log = getLogging();

	if (log) {
		strcpy(LOG_STATE, STR_LOG_ENABLED);
	} else {
		strcpy(LOG_STATE, STR_LOG_DISABLED);
	}
}

const char* date() {
	return "=2026/02/26 12:42=";
}

void UI_Draw3Rows() {

	ssd1306_Fill(Black);

	ssd1306_SetCursor(0, 0);
	ssd1306_WriteString((char*) row[0], Font_7x10, White);
//Font_7x10
	ssd1306_SetCursor(0, 10);
	ssd1306_WriteString((char*) row[1], Font_7x10, White);

	ssd1306_SetCursor(0, 21);
	ssd1306_WriteString((char*) row[2], Font_7x10, White);

	ssd1306_UpdateScreen();
}

void r_Disp_main() {

	snprintf(row[0], ROW_SIZE, "%s", date());
	snprintf(row[1], ROW_SIZE, "%s", STR_MEAS_HEADER);
	snprintf(row[2], ROW_SIZE, "%s", STR_PRESS_DISPLAY);
	UI_Draw3Rows();
}

void r_Disp_1() {

	snprintf(row[0], ROW_SIZE, "%s", date());
	str_dist_OLED(row[1], ROW_SIZE);
	str_temp_OLED(row[2], ROW_SIZE);
	UI_Draw3Rows();
}

void r_Disp_2() {

	snprintf(row[0], ROW_SIZE, "%s", date());
	str_Accel_OLED(row[1],ROW_SIZE);
	str_LUX_OLED(row[2],ROW_SIZE);
	UI_Draw3Rows();
}

void r_Disp_3() {

	snprintf(row[0], ROW_SIZE, "%s", date());
	str_LAT_LONG_OLED(row[1], row[2], ROW_SIZE);
	UI_Draw3Rows();
}

void r_Disp_4() {
	snprintf(row[0], ROW_SIZE, "%s", date());
	str_SPEED_HEAD_OLED(row[1], row[2],ROW_SIZE);
	UI_Draw3Rows();
}

void r_Disp_5() {
	snprintf(row[0], ROW_SIZE, "Fuel Efficiency:");
	str_FuelEfficiency_OLED(row[1], row[2], ROW_SIZE);
	UI_Draw3Rows();
}

void r_Data_main() {
	snprintf(row[0], ROW_SIZE, "%s", date());
	snprintf(row[1], ROW_SIZE, "%s", STR_DATA_HEADER);
	snprintf(row[2], ROW_SIZE, "%s", STR_PRESS_DISPLAY);
	UI_Draw3Rows();
}

void r_Data_1() {

	snprintf(row[0], ROW_SIZE, "Enter fuel liters");
	str_fuel_OLED(row[1],ROW_SIZE);
	snprintf(row[2], ROW_SIZE, "%s", S3_STATE);
	UI_Draw3Rows();
}

void r_Data_2() {

	snprintf(row[0], ROW_SIZE, "Enter odometer km");
	str_dist_ODO_OLED(row[1],ROW_SIZE);
	snprintf(row[2], ROW_SIZE, "%s", S3_STATE);
	UI_Draw3Rows();
}

void r_Data_3() {
	str_toggleLOG();

	snprintf(row[0], ROW_SIZE, "Log data (Y/N)");
	snprintf(row[1], ROW_SIZE, "'*' = Y / '#' = N");
	snprintf(row[2], ROW_SIZE, "%s", LOG_STATE);
	UI_Draw3Rows();
}

void r_Diag_main() {


	snprintf(row[0], ROW_SIZE, "%s", date());
	snprintf(row[1], ROW_SIZE, "%s", STR_DIAG_HEADER);
	snprintf(row[2], ROW_SIZE, "%s", STR_PRESS_DISPLAY);
	UI_Draw3Rows();
}

void r_Diag_1() {

	snprintf(row[0], ROW_SIZE, "%s", date());
	snprintf(row[1], ROW_SIZE, "%s", STR_DIAG_SUB_HEADER);

	str_toggleSD_State(getSD_OK());

	snprintf(row[2], ROW_SIZE, "%s", SD_DIAG);
	UI_Draw3Rows();
}

void r_Diag_2() {

	snprintf(row[0], ROW_SIZE, "%s", date());
	snprintf(row[1], ROW_SIZE, "%s", STR_DIAG_SUB_HEADER);

	str_toggleMPU_State(getMPU_OK());

	snprintf(row[2], ROW_SIZE, "%s", MPU_DIAG);
	UI_Draw3Rows();
}

void r_Diag_3() {

	snprintf(row[0], ROW_SIZE, "%s", date());
	snprintf(row[1], ROW_SIZE, "%s", STR_DIAG_SUB_HEADER);

	str_toggleGPS_State(getGPS_OK());

	snprintf(row[2], ROW_SIZE, "%s", GPS_DIAG);
	UI_Draw3Rows();
}

void r_Diag_4() {

	str_toggleLOG();
	snprintf(row[0], ROW_SIZE, "%s", date());
	snprintf(row[1], ROW_SIZE, "%s", STR_DIAG_SUB_HEADER);

	snprintf(row[2], ROW_SIZE, "%s", LOG_STATE);
	UI_Draw3Rows();
}

void r_Warn_UnsafeDriving() {

	snprintf(row[0], ROW_SIZE, "%s", STR_WARN_HEADER);
	snprintf(row[1], ROW_SIZE, "--Unsafe Driving--");
	str_Accel_OLED(row[2],ROW_SIZE);
	UI_Draw3Rows();
}

void r_Warn_Proximity() {

	snprintf(row[0], ROW_SIZE, "%s", STR_WARN_HEADER);
	snprintf(row[1], ROW_SIZE, "----Proximity-----");
	str_dist_OLED(row[2],ROW_SIZE);
	UI_Draw3Rows();
}

void r_Warn_Light() {

	snprintf(row[0], ROW_SIZE, "%s", STR_WARN_HEADER);
	snprintf(row[1], ROW_SIZE, "----Low Light-----");
	str_LUX_OLED(row[2],ROW_SIZE);
	UI_Draw3Rows();
}

void r_Warn_Temp() {

	snprintf(row[0], ROW_SIZE, "%s", STR_WARN_HEADER);
	snprintf(row[1], ROW_SIZE, "-High Temperature-");
	str_temp_OLED(row[2],ROW_SIZE);
	UI_Draw3Rows();
}

void r_Warn_Impact() {

	snprintf(row[0], ROW_SIZE, "%s", STR_WARN_HEADER);
	snprintf(row[1], ROW_SIZE, "-Impact Detected--");
	str_Accel_OLED(row[2],ROW_SIZE);
	UI_Draw3Rows();
}

void init_OLED() {
	ssd1306_Init();
}

void Menu_Init() {

	//Warning
	Warn_UnsafeDriving.render = r_Warn_UnsafeDriving;
	Warn_Impact.render = r_Warn_Impact;
	Warn_Light.render = r_Warn_Light;
	Warn_Proximity.render = r_Warn_Proximity;
	Warn_Temp.render = r_Warn_Temp;
	//Display_main
	Display_main.parent = NULL;
	Display_main.child = &Display_1;
	Display_main.up = &Data_main;
	Display_main.down = &Diagnostics_main;
	Display_main.render = r_Disp_main;

	Display_1.parent = &Display_main;
	Display_1.child = NULL;
	Display_1.up = &Display_2;
	Display_1.down = &Display_5;
	Display_1.render = r_Disp_1;

	Display_2.parent = &Display_main;
	Display_2.child = NULL;
	Display_2.up = &Display_3;
	Display_2.down = &Display_1;
	Display_2.render = r_Disp_2;

	Display_3.parent = &Display_main;
	Display_3.child = NULL;
	Display_3.up = &Display_4;
	Display_3.down = &Display_2;
	Display_3.render = r_Disp_3;

	Display_4.parent = &Display_main;
	Display_4.child = NULL;
	Display_4.up = &Display_5;
	Display_4.down = &Display_3;
	Display_4.render = r_Disp_4;

	Display_5.parent = &Display_main;
	Display_5.child = NULL;
	Display_5.up = &Display_1;
	Display_5.down = &Display_4;
	Display_5.render = r_Disp_5;

	//Data
	Data_main.parent = NULL;
	Data_main.child = &Data_1;
	Data_main.up = &Diagnostics_main;
	Data_main.down = &Display_main;
	Data_main.render = r_Data_main;

	Data_1.parent = &Data_main;
	Data_1.child = NULL;
	Data_1.up = &Data_2;
	Data_1.down = &Data_3;
	Data_1.render = r_Data_1;

	strcpy(S3_STATE, STR_S3_CHANGE);

	Data_2.parent = &Data_main;
	Data_2.child = NULL;
	Data_2.up = &Data_3;
	Data_2.down = &Data_1;
	Data_2.render = r_Data_2;

	Data_3.parent = &Data_main;
	Data_3.child = NULL;
	Data_3.up = &Data_1;
	Data_3.down = &Data_2;
	Data_3.render = r_Data_3;

	strcpy(LOG_STATE, STR_LOG_DISABLED);

	//Diagnostics

	Diagnostics_main.parent = NULL;
	Diagnostics_main.child = &Diagnostics_1;
	Diagnostics_main.up = &Display_main;
	Diagnostics_main.down = &Data_main;
	Diagnostics_main.render = r_Diag_main;

	Diagnostics_1.parent = &Diagnostics_main;
	Diagnostics_1.child = NULL;
	Diagnostics_1.up = &Diagnostics_2;
	Diagnostics_1.down = &Diagnostics_4;
	Diagnostics_1.render = r_Diag_1;

	Diagnostics_2.parent = &Diagnostics_main;
	Diagnostics_2.child = NULL;
	Diagnostics_2.up = &Diagnostics_3;
	Diagnostics_2.down = &Diagnostics_1;
	Diagnostics_2.render = r_Diag_2;

	Diagnostics_3.parent = &Diagnostics_main;
	Diagnostics_3.child = NULL;
	Diagnostics_3.up = &Diagnostics_4;
	Diagnostics_3.down = &Diagnostics_2;
	Diagnostics_3.render = r_Diag_3;

	Diagnostics_4.parent = &Diagnostics_main;
	Diagnostics_4.child = NULL;
	Diagnostics_4.up = &Diagnostics_1;
	Diagnostics_4.down = &Diagnostics_3;
	Diagnostics_4.render = r_Diag_4;

}

