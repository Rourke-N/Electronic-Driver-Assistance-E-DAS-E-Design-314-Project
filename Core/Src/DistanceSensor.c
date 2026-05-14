#include "DistanceSensor.h"
#include "General.h"

#define TRIG_TIME 10
#define MAX_DISTANCE_TIME 36
const float MAX_DISTANCE = 99.9f;
const float D_CONVERT = 1.0f / 58.0f;
volatile uint8_t D_ready = 1;
uint32_t d_trig_tick = 0;
volatile uint32_t start_count = 0;
volatile uint32_t finish_count = 0;
volatile uint8_t D_RISING;
volatile uint8_t counting_distance;
volatile uint32_t d_last_read = 0xFFFF;
uint32_t trig_down_tick;
uint32_t start_trigger_count = 0;

//Warning
#define PROXIMITY_DISTANCE 10
#define RELIEF_DISTANCE 15
uint8_t proximity_warning = 0;

#define D_INTERVAL 20

float current_distance = 99.9f;

char str_dist[10];

void clearProximityWarning() {
	proximity_warning = 0;
}
void update_str_dist() {

	float distance = current_distance;

	uint32_t d_whole;
	uint32_t d_decimal;

	WholeFraction(distance, 1, &d_whole, &d_decimal);

	// Safe formatting into the local global str_dist[10]
	snprintf(str_dist, sizeof(str_dist), "%02lu.%1lu cm", d_whole, d_decimal);
}

// Format: xx.x  (no sign, one decimal place)
void str_Dist_SD(char *dest, size_t size) {
	uint32_t whole, decimal;
	WholeFraction(getDistance(), 1, &whole, &decimal);
	snprintf(dest, size, "%lu.%01lu", whole, decimal);
}

void str_dist_UART(char *dest, size_t size) {

	update_str_dist();

	// Using an offset to append to the end of the existing string safely
	size_t len = strlen(dest);
	if (len < size) {
		snprintf(dest + len, size - len, "Distance:    %s\n", str_dist);
	}
}

void str_dist_OLED(char *dest, size_t size) {

	update_str_dist();

	// Standard safe format into the OLED buffer
	snprintf(dest, size, "Dist       %s", str_dist);
}

float getDistance() {
	return current_distance;
}

void sampleDistanceSensor() {

	if ((!counting_distance && HAL_GetTick() - d_last_read > D_INTERVAL)
			|| (HAL_GetTick() - trig_down_tick > MAX_DISTANCE_TIME)) {
		D_ready = 1;
		counting_distance = 1;
		start_trigger_count = TIM4->CNT;
	}

	if (D_ready) {
		HAL_GPIO_WritePin(GPIOC, TRIG_Pin, GPIO_PIN_SET);
		D_RISING = 1;
		D_ready = 0;
		trig_down_tick = HAL_GetTick();
	}
	if ( TIM4->CNT - start_trigger_count > TRIG_TIME) {
		HAL_GPIO_WritePin(GPIOC, TRIG_Pin, GPIO_PIN_RESET);
		//TIM4->CNT = 0;
	}
}

uint8_t getProximityWarning() {
	if (current_distance <= PROXIMITY_DISTANCE) {
		proximity_warning = 1;
	} else if (proximity_warning && current_distance > RELIEF_DISTANCE) {
		proximity_warning = 0;
	}
	return proximity_warning;
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {

	if (htim->Instance == TIM4) {
		if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7) == GPIO_PIN_SET) {
			//start_count = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
			TIM4->CNT = 0;
			start_count = 0;
		} else {
			finish_count = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
			counting_distance = 0;
			uint32_t d_elapsed = finish_count - start_count;
			current_distance = d_elapsed * D_CONVERT;
			if (current_distance > MAX_DISTANCE) {
				current_distance = MAX_DISTANCE;
			}
			d_last_read = HAL_GetTick();
		}
	}

//sprintf(tx_buffer, "Timer triggered\r\n");
//	HAL_UART_Transmit(&huart2, (uint8_t*) tx_buffer, strlen(tx_buffer), 1000);

}
