#include "TempSensor.h"

#define T_SAMPLE_SIZE 7
#define PULSE_TRAIN_LENGTH 60
const float UNCOMFORTABLE_TEMP = 24.0f;
const float T_CONVERT = 256.0f / 4096.0f;

uint32_t first_pulse_tick = 0;
uint32_t pulse_count;
uint8_t counting_temp = 0;

float averaged_tempC;
float raw_tempC;

float t_samples[T_SAMPLE_SIZE] = { 0 };
uint8_t t_sample_index = 0;

//ALARM
const uint32_t TEMP_WARN_DELAY = 1000;
uint8_t temp_alarm_enabled = 0;
uint8_t high_temp_warning = 0;
int lastTempWarning = 0;

extern volatile uint32_t *LEDs[];

char str_temp[10];

void update_str_temp() {
	float temp = getTemp();
	uint32_t t_whole;
	uint32_t t_decimal;
	char t_sign = sign(temp);
	WholeFraction(temp, 1, &t_whole, &t_decimal);
	sprintf(str_temp, "%c%02lu.%1lu C", t_sign, t_whole, t_decimal);
}

void override_setTemp(uint8_t set) { //
	if (set) {
		high_temp_warning = 1;
		disableTempAlarmCheck();
	} else {
		enableTempAlarmCheck();
	}

}

void state_temp_alarm() {
	flashLED(D5);
}

void str_temp_OLED(char *dest) {
	update_str_temp();
	sprintf(dest, "Temp:       %s", str_temp);
}

void str_temp_UART(char *dest) {
	update_str_temp();
	sprintf(dest + strlen(dest), "Temperature: %s\n", str_temp);
}

float getTemp() {
	return averaged_tempC;
}

void enableTempAlarmCheck() {
	temp_alarm_enabled = 1;
}

void disableTempAlarmCheck() {
	temp_alarm_enabled = 0;
	high_temp_warning = 0;
}

void clearTempWarning(uint8_t delay){
	high_temp_warning = 0;
	if(delay){
		lastTempWarning = HAL_GetTick();
	}
}

uint8_t getTempWarning() {

	//if (temp_alarm_enabled && HAL_GetTick() - lastTempWarning > TEMP_WARN_DELAY) {
		if (averaged_tempC > UNCOMFORTABLE_TEMP) {
			high_temp_warning = 1;
		} else { //Can only clear if it was set here
			high_temp_warning = 0;
		}
	//}

	return high_temp_warning;
}

void sampleTempSensor() {

	pulse_count = TIM2->CNT;

	if (pulse_count > 0 && !counting_temp) {
		first_pulse_tick = HAL_GetTick();
		counting_temp = 1;
	}
	if (counting_temp
			&& (HAL_GetTick() - first_pulse_tick >= PULSE_TRAIN_LENGTH)) {

		raw_tempC = (pulse_count * T_CONVERT) - 50.0f;

		t_samples[t_sample_index] = raw_tempC;

		counting_temp = 0;
		pulse_count = 0;
		t_sample_index += 1;
		TIM2->CNT = 0;
	}

	if (t_sample_index == T_SAMPLE_SIZE) {

		float min = 0;
		float max = 0;
		float total = 0;

		for (int i = 0; i < T_SAMPLE_SIZE; i++) {

			total += t_samples[i];

			if (i == 0) {
				min = t_samples[i];
				max = t_samples[i];
			} else if (t_samples[i] > max) {
				max = t_samples[i];
			} else if (t_samples[i] < min) {
				min = t_samples[i];
			}
		}

		averaged_tempC = (total - min - max) / (T_SAMPLE_SIZE - 2.0f);
		t_sample_index = 0;
	}
}

