#include "TempSensor.h"
#include "myRTC.h"

#define T_SAMPLE_SIZE 7
#define PULSE_TRAIN_LENGTH 60
const float TEMP_CONST = 50.0f;
#define TEMP_DELAY 30

const float MAX_TEMP = 99.9f;

const float UNCOMFORTABLE_TEMP = 30.0f;
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

// Replace warning timing variables with these
static uint8_t temp_suppressed = 0;
static RTC_TimeTypeDef temp_cleared_time = {0};
static RTC_DateTypeDef temp_cleared_date = {0};

// Same helper — defined in LightSensor.c, declare extern in TempSensor.h
// or duplicate it here. Duplicating is simpler to avoid header changes:

static uint32_t tempMinutesSince(RTC_TimeTypeDef *thenTime,
		RTC_DateTypeDef *thenDate) {
	extern RTC_HandleTypeDef hrtc;
	RTC_TimeTypeDef nowTime = { 0 };
	RTC_DateTypeDef nowDate = { 0 };

	HAL_RTC_GetTime(&hrtc, &nowTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &nowDate, RTC_FORMAT_BIN);

	// If year or month is higher, elapsed time is definitely beyond any warning delay
	if (nowDate.Year > thenDate->Year || nowDate.Month > thenDate->Month) {
		return 0xFFFFFFFF;
	}

	int32_t dayDiff = (int32_t) nowDate.Date - (int32_t) thenDate->Date;
	int32_t nowMins = dayDiff * 1440 + nowTime.Hours * 60 + nowTime.Minutes;
	int32_t thenMins = thenTime->Hours * 60 + thenTime->Minutes;

	int32_t elapsed = nowMins - thenMins;

	return (elapsed >= 0) ? (uint32_t) elapsed : 0;
}

void update_str_temp() {
    float temp = getTemp();
    uint32_t t_whole;
    uint32_t t_decimal;
    char t_sign = sign(temp);

    WholeFraction(temp, 1, &t_whole, &t_decimal);

    // str_temp is [10]. "+23.5 C" uses 7 characters + null terminator.
    snprintf(str_temp, sizeof(str_temp), "%c%02lu.%1lu C", t_sign, t_whole, t_decimal);
}

void str_temp_OLED(char *dest, size_t size) {
    update_str_temp();

    // "Temp:      +23.5 C" is exactly 18 characters.
    snprintf(dest, size, "Temp:      %s", str_temp);
}

// Format: ±xx.x  (sign is '-' or nothing, one decimal place)
void str_Temp_SD(char *dest, size_t size) {
	float temp = getTemp();
	char s = (temp < 0) ? '-' : ' ';
	uint32_t whole, decimal;
	WholeFraction(temp, 1, &whole, &decimal);
	// Strip the leading space for positive — PDD example has no space
	if (s == ' ') {
		snprintf(dest, size, "%lu.%01lu", whole, decimal);
	} else {
		snprintf(dest, size, "-%lu.%01lu", whole, decimal);
	}
}


void str_temp_UART(char *dest, size_t size) {
    update_str_temp();

    size_t len = strlen(dest);
    if (len < size) {
        // "Temperature: +23.5 C\n"
        // 13 (label) + 7 (value) + 1 (newline) = 21 characters.
        snprintf(dest + len, size - len, "Temperature: %s\n", str_temp);
    }
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


float getTemp() {
	if (averaged_tempC > fabs(MAX_TEMP)) {
		return MAX_TEMP;
	} else {
		return averaged_tempC;
	}
}

void enableTempAlarmCheck() {
	temp_alarm_enabled = 1;
}

void disableTempAlarmCheck() {
	temp_alarm_enabled = 0;
	high_temp_warning = 0;
}

void clearTempWarning() {
    extern RTC_HandleTypeDef hrtc;
    high_temp_warning = 0;

        // Record when the warning was cleared
        // getTempWarning() suppresses for 30 minutes from this point
        temp_suppressed = 1;
        HAL_RTC_GetTime(&hrtc, &temp_cleared_time, RTC_FORMAT_BIN);
        HAL_RTC_GetDate(&hrtc, &temp_cleared_date, RTC_FORMAT_BIN);
}

uint8_t getTempWarning() {
    // If suppressed, check if 30 minutes have elapsed
    if (temp_suppressed) {
        if (tempMinutesSince(&temp_cleared_time, &temp_cleared_date) >= TEMP_DELAY) {
            temp_suppressed = 0;  // Cooldown expired
        } else {
            return 0;  // Still in cooldown
        }
    }

    if (averaged_tempC > UNCOMFORTABLE_TEMP) {
        high_temp_warning = 1;
    } else {
        high_temp_warning = 0;
    }

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

		raw_tempC = (pulse_count * T_CONVERT) - TEMP_CONST;

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

