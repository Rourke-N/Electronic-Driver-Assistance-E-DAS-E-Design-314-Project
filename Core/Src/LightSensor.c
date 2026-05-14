#include "LightSensor.h"
#include "General.h"
#include "myRTC.h"

uint32_t current_light = 0;
uint8_t light_warning = 0;

char str_light[10];

extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;

volatile uint16_t SensorBuffer[10] = { 0 };

const float conversion_factor = 0.2197; //4096*conv = 900

static uint8_t light_suppressed = 0;  // 1 = in cooldown, can't trigger
static RTC_TimeTypeDef light_cleared_time = { 0 };
static RTC_DateTypeDef light_cleared_date = { 0 };

#define LIGHT_DELAY 60

static uint32_t minutesSince(RTC_TimeTypeDef *thenTime,
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

void str_Light_SD(char *dest, size_t size) {
	snprintf(dest, size, "%04lu", getLight());
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
    // If currently suppressed, check if 60 minutes have elapsed
    if (light_suppressed) {
        if (minutesSince(&light_cleared_time, &light_cleared_date) >= LIGHT_DELAY) {
            light_suppressed = 0;  // Cooldown expired, allow warnings again
        } else {
            return 0;  // Still in cooldown — never update light_warning high
        }
    }

    if (current_light < 300) {
        light_warning = 1;
    } else if (current_light > 400) {
        light_warning = 0;
    }

    return light_warning;
}

void clearLightWarning() {
	extern RTC_HandleTypeDef hrtc;
	light_warning = 0;
	// Record the exact RTC time the warning was cleared
	// getLightWarning() will suppress for 60 minutes from this point
	light_suppressed = 1;
	HAL_RTC_GetTime(&hrtc, &light_cleared_time, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &light_cleared_date, RTC_FORMAT_BIN);

}
