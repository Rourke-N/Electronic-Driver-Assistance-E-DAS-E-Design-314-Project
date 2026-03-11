#include "General.h"

//Date
#define YEAR 2026
uint8_t month = 3;
uint8_t day = 3;
uint8_t hour = 10;
uint8_t minute = 10;
uint8_t second = 10;

//LEDS
uint8_t D3_ON = 1;
uint8_t D5_ON = 1;

//BUTTONS
volatile int triggerTick[6] = { 0 };
volatile uint8_t triggerDetected[6] = { 0 }; //

//UART
const char START_CHAR = '@';
const char END_CHAR = '&';
extern UART_HandleTypeDef huart2;
uint8_t rx_byte;
char command_str[50];
uint8_t command_index = 0;
volatile uint8_t transmitting_message = 0;
uint8_t message_ready = 0;
uint8_t command_ready;
char g_tx_buffer[50];
#define MESSAGE_LENGTH 22
#define MAX_TRANSMISSION 100

//Timers
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim3;

void handleCommand() {

	if (strcmp(command_str, "Stat") == 0) {
		HAL_UART_Transmit(&huart2, (uint8_t*) &START_CHAR, 1, MAX_TRANSMISSION);
		displayDate();
		displayDistance(getDistance());
		displayTemp(getTemp());
		displayLight(getLight());
		displayAccel(getX(), getY(), getZ());
		displayAlarmConditions();
		displayGPS(getLat(), getLong());
		HAL_UART_Transmit(&huart2, (uint8_t*) "&\n", 2, MAX_TRANSMISSION);
	}
}

void displayDate() {

	sprintf(g_tx_buffer, "%04u/%02u/%02u %02u:%02u:%02u \n",
	YEAR, month, day, hour, minute, second);
	HAL_UART_Transmit(&huart2, (uint8_t*) g_tx_buffer, MESSAGE_LENGTH - 1,
	MAX_TRANSMISSION);
}

void displayGPS(float gps_lat, float gps_long) {

	uint32_t lat_whole;
	uint32_t lat_decimal;
	char lat_sign = sign(gps_lat);

	uint32_t long_whole;
	uint32_t long_decimal;
	char long_sign = sign(gps_long);

	WholeFraction(gps_lat, 5, &lat_whole, &lat_decimal);
	WholeFraction(gps_long, 5, &long_whole, &long_decimal);

	sprintf(g_tx_buffer, "GPS lat:   %c%03lu.%5lu\n", lat_sign, lat_whole,
			lat_decimal);
	HAL_UART_Transmit(&huart2, (uint8_t*) g_tx_buffer, MESSAGE_LENGTH, 100);

	sprintf(g_tx_buffer, "GPS long:  %c%03lu.%5lu\n", long_sign, long_whole,
			long_decimal);
	HAL_UART_Transmit(&huart2, (uint8_t*) g_tx_buffer, MESSAGE_LENGTH, 100);
}

void displayAlarmConditions() {

	char unsafe_driving = YesNo(getUnsafeDriving()); //accelerometer
	char impact = YesNo(getImpact());                //accelerometer
	char low_light_warning = YesNo(getLowLight());           //photodiode
	char proximity_warning = YesNo(getProximityWarning());
	char high_temp = YesNo(getTempWarning());

	sprintf(g_tx_buffer, "Unsafe driving:     %c\n", unsafe_driving);
	HAL_UART_Transmit(&huart2, (uint8_t*) g_tx_buffer, MESSAGE_LENGTH,
	MAX_TRANSMISSION);

	sprintf(g_tx_buffer, "Impact detected:    %c\n", impact);
	HAL_UART_Transmit(&huart2, (uint8_t*) g_tx_buffer, MESSAGE_LENGTH,
	MAX_TRANSMISSION);

	sprintf(g_tx_buffer, "Low-Light warning:  %c\n", low_light_warning);
	HAL_UART_Transmit(&huart2, (uint8_t*) g_tx_buffer, MESSAGE_LENGTH,
	MAX_TRANSMISSION);

	sprintf(g_tx_buffer, "Proximity warning:  %c\n", proximity_warning);
	HAL_UART_Transmit(&huart2, (uint8_t*) g_tx_buffer, MESSAGE_LENGTH,
	MAX_TRANSMISSION);

	sprintf(g_tx_buffer, "High Temperature:   %c\n", high_temp);
	HAL_UART_Transmit(&huart2, (uint8_t*) g_tx_buffer, MESSAGE_LENGTH,
	MAX_TRANSMISSION);

}

void displayTemp(float temp) {

	uint32_t t_whole;
	uint32_t t_decimal;
	char t_sign = sign(temp);

	WholeFraction(temp, 1, &t_whole, &t_decimal);

	sprintf(g_tx_buffer, "Temperature:  %c%02lu.%1lu C\n", t_sign, t_whole,
			t_decimal);
	HAL_UART_Transmit(&huart2, (uint8_t*) g_tx_buffer, MESSAGE_LENGTH, 100);
}

void displayDistance(float distance) {
	uint32_t d_whole;
	uint32_t d_decimal;

	WholeFraction(distance, 1, &d_whole, &d_decimal);

	sprintf(g_tx_buffer, "Distance:     %02lu.%1lu cm\n", d_whole, d_decimal);
	HAL_UART_Transmit(&huart2, (uint8_t*) g_tx_buffer, MESSAGE_LENGTH, 100);
}

void displayLight(uint32_t light) {
	sprintf(g_tx_buffer, "Light:       %04lu lux\n", light);
	HAL_UART_Transmit(&huart2, (uint8_t*) g_tx_buffer, MESSAGE_LENGTH, 100); //22 to include newline
}

void displayAccel(float x, float y, float z) {
	char x_sign = sign(x);
	char y_sign = sign(y);
	char z_sign = sign(z);

	uint32_t x_whole, y_whole, z_whole;
	uint32_t x_decimal, y_decimal, z_decimal;

	WholeFraction(x, 2, &x_whole, &x_decimal);
	WholeFraction(y, 2, &y_whole, &y_decimal);
	WholeFraction(z, 2, &z_whole, &z_decimal);

	sprintf(g_tx_buffer, "X accel:      %c%lu.%02lu g\n", x_sign, x_whole,
			x_decimal);
	HAL_UART_Transmit(&huart2, (uint8_t*) g_tx_buffer, MESSAGE_LENGTH, 100);

	sprintf(g_tx_buffer, "Y accel:      %c%lu.%02lu g\n", y_sign, y_whole,
			y_decimal);
	HAL_UART_Transmit(&huart2, (uint8_t*) g_tx_buffer, MESSAGE_LENGTH, 100);

	sprintf(g_tx_buffer, "X accel:      %c%lu.%02lu g\n", z_sign, z_whole,
			z_decimal);
	HAL_UART_Transmit(&huart2, (uint8_t*) g_tx_buffer, MESSAGE_LENGTH, 100);
}
void WholeFraction(float value, uint8_t precision, uint32_t *whole,
		uint32_t *decimal) {
//Returns absolute value

	float offset = 0.5f;
	float multiplier = 1.0f;

	for (uint8_t i = 0; i < precision; i++) {
		multiplier *= 10.0f;
		offset /= 10.0f;
	}
	float absolute_val = fabs(value);
	float rounded = absolute_val + offset; // Apply the push once
	*whole = (int) rounded;
	*decimal = (int) ((rounded - (float) *whole) * multiplier); //
}

char sign(float value) {
	if (value > 0) {
		return '+';
	} else {
		return '-';
	}
}

char YesNo(uint8_t value) {
	if (value > 0) {
		return 'y';
	} else {
		return 'n';
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {

	if (GPIO_Pin == MIDDLE_BUTTON_Pin) {
		//|| GPIO_Pin == BUTTON_Pin
		triggerDetected[MIDDLE] = 1;
		triggerTick[MIDDLE] = HAL_GetTick();
	} else if (GPIO_Pin == UP_BUTTON_Pin) {
		triggerDetected[UP] = 1;
		triggerTick[UP] = HAL_GetTick();
	} else if (GPIO_Pin == DOWN_BUTTON_Pin) {
		triggerDetected[DOWN] = 1;
		triggerTick[DOWN] = HAL_GetTick();
	} else if (GPIO_Pin == LEFT_BUTTON_Pin) {
		triggerDetected[LEFT] = 1;
		triggerTick[LEFT] = HAL_GetTick();
	} else if (GPIO_Pin == RIGHT_BUTTON_Pin) {
		triggerDetected[RIGHT] = 1;
		triggerTick[RIGHT] = HAL_GetTick();
	}
}

void handleButton(ButtonIndex btn) {
	switch (btn) {
	case MIDDLE:
		if (HAL_GPIO_ReadPin(GPIOB, MIDDLE_BUTTON_Pin) == 1) {
//|| (HAL_GPIO_ReadPin(GPIOC, BUTTON_Pin
			HAL_UART_Transmit(&huart2, (uint8_t*) &START_CHAR, 1,
					MAX_TRANSMISSION);
			displayDate();
			displayDistance(getDistance());
			displayTemp(getTemp());
			displayLight(getLight());
			displayAccel(getX(), getY(), getZ());
			displayAlarmConditions();
			displayGPS(getLat(), getLong());
			HAL_UART_Transmit(&huart2, (uint8_t*) "&\n", 2, MAX_TRANSMISSION);
			triggerDetected[MIDDLE] = 0;
		}
		break;
	case UP:
		if (HAL_GPIO_ReadPin(GPIOB, UP_BUTTON_Pin) == 1) {

			HAL_GPIO_TogglePin(GPIOA, D2_Pin);

			//sprintf(tx_buffer, "Up Button pressed\r\n");
			//HAL_UART_Transmit(&huart2, (uint8_t*) tx_buffer, strlen(tx_buffer), 1000);
			triggerDetected[UP] = 0;
		}
		break;
	case DOWN:
		if (HAL_GPIO_ReadPin(GPIOB, DOWN_BUTTON_Pin) == 1) {
			if (D5_ON) {
				HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
				D5_ON = 0;
			} else {
				HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
				D5_ON = 1;
			}
			//sprintf(tx_buffer, "Down Button pressed\r\n");
			//HAL_UART_Transmit(&huart2, (uint8_t*) tx_buffer, strlen(tx_buffer), 1000);
			triggerDetected[DOWN] = 0;
		}
		break;
	case LEFT:
		if (HAL_GPIO_ReadPin(GPIOA, LEFT_BUTTON_Pin) == 1) {

			if (D3_ON) {
				HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
				D3_ON = 0;
			} else {
				HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
				D3_ON = 1;
			}

			//sprintf(tx_buffer, "Left Button pressed\r\n");
			//HAL_UART_Transmit(&huart2, (uint8_t*) tx_buffer, strlen(tx_buffer), 1000);
			triggerDetected[LEFT] = 0;
		}
		break;
	case RIGHT:
		if (HAL_GPIO_ReadPin(GPIOA, RIGHT_BUTTON_Pin) == 1) {

			HAL_GPIO_TogglePin(GPIOA, D4_Pin);

			//sprintf(tx_buffer, "Right Button pressed\r\n");
			//HAL_UART_Transmit(&huart2, (uint8_t*) tx_buffer, strlen(tx_buffer), 1000);
			triggerDetected[RIGHT] = 0;
		}
		break;

	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART2) {

		if (rx_byte == START_CHAR) {
			//HAL_GPIO_TogglePin(GPIOA, D2_Pin);

			transmitting_message = 1;
			command_index = 0;
		} else if (rx_byte == '\n' && !message_ready) {
			command_index = 0;
			transmitting_message = 0;
		}

		else if (rx_byte == END_CHAR && transmitting_message) {
			command_str[command_index] = '\0';
			transmitting_message = 0;
			message_ready = 1;
		} else if (rx_byte == '\n' && message_ready) {
			command_ready = 1;
			message_ready = 0;
		} else if (transmitting_message) {
			command_str[command_index] = rx_byte;
			command_index += 1;
		}

		// CRITICAL: You must call this again to listen for the NEXT byte
		HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
	}
}

void defaultSetup() {

	HAL_UART_Receive_IT(&huart2, &rx_byte, 1);

	HAL_GPIO_WritePin(GPIOA, D2_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, D4_Pin, GPIO_PIN_SET);

	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);

}
