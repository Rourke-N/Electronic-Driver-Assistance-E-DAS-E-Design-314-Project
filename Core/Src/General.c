#include "General.h"

//Date
#define YEAR 2026
uint8_t month = 3;
uint8_t day = 3;
uint8_t hour = 10;
uint8_t minute = 10;
uint8_t second = 10;

//LEDS
#define LED_FLASHING 1000
#define LED_OFF 0
#define LED_ON 2000

volatile uint32_t *LEDs[] = { &TIM3->CCR4, // D2
		&TIM3->CCR1, // D3
		&TIM3->CCR2, // D4
		&TIM1->CCR2  // D5
		};

//BUTTONS
volatile uint32_t triggerTick[6] = { 0 };
volatile uint8_t triggerDetected[6] = { 0 };
#define DEBOUNCE_TIME 25

//KEYPAD
volatile uint32_t rowTick[4] = { 0 };
volatile uint8_t rowDetected[4] = { 0 };

//UART
const char START_CHAR = '@';
const char END_CHAR = '&';
extern UART_HandleTypeDef huart2;

volatile uint8_t rx_byte;
volatile char command_str[50];
volatile uint8_t command_index = 0;

volatile uint8_t transmitting_message = 0;
volatile uint8_t message_ready = 0;
volatile uint8_t command_ready;

char g_tx_buffer[150];
#define MESSAGE_LENGTH 21
#define MAX_TRANSMISSION 100
static char display_buffer[400];

//Timers
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim3;

void handleCommand() {

	display_buffer[0] = '\0';

	if (strcmp(command_str, "Stat") == 0) {

		//HAL_UART_Transmit(&huart2, (uint8_t*) &START_CHAR, 1, MAX_TRANSMISSION);
		sprintf(display_buffer + strlen(display_buffer), "%c", START_CHAR);
		displayDate(display_buffer);
		displayDistance(display_buffer);
		displayTemp(display_buffer);
		displayLight(display_buffer);
		displayAccel(display_buffer);
		displayAlarmConditions(display_buffer);
		displayGPS(display_buffer);
		sprintf(display_buffer + strlen(display_buffer), "%c\n", END_CHAR);
		HAL_UART_Transmit_IT(&huart2, (uint8_t*) display_buffer,
				strlen(display_buffer));
		//HAL_UART_Transmit(&huart2, (uint8_t*) display_buffer,strlen(display_buffer), MAX_TRANSMISSION);
	}
}

void displayDate(char *dest) {

	sprintf(dest + strlen(dest), "%04u/%02u/%02u %02u:%02u:%02u \n",
	YEAR, month, day, hour, minute, second);
	//HAL_UART_Transmit(&huart2, (uint8_t*) g_tx_buffer, MESSAGE_LENGTH, MAX_TRANSMISSION);
}

void displayGPS(char *dest) {

	float gps_lat = getLat();
	float gps_long = getLong();

	uint32_t lat_whole;
	uint32_t lat_decimal;
	char lat_sign = sign(gps_lat);

	uint32_t long_whole;
	uint32_t long_decimal;
	char long_sign = sign(gps_long);

	WholeFraction(gps_lat, 5, &lat_whole, &lat_decimal);
	WholeFraction(gps_long, 5, &long_whole, &long_decimal);

	sprintf(dest + strlen(dest), "GPS lat:  %c%03lu.%5lu\n", lat_sign,
			lat_whole, lat_decimal);
	//HAL_UART_Transmit(&huart2, (uint8_t*) g_tx_buffer, MESSAGE_LENGTH, 100);

	sprintf(dest + strlen(dest), "GPS long: %c%03lu.%5lu\n", long_sign,
			long_whole, long_decimal);
	//HAL_UART_Transmit(&huart2, (uint8_t*) g_tx_buffer, MESSAGE_LENGTH, 100);
}

void displayAlarmConditions(char *dest) {

	uint8_t unsafe_driving = getUnsafeDriving(); //accelerometer
	uint8_t impact = getImpact();                //accelerometer
	uint8_t low_light_warning = getLowLight();           //photodiode
	uint8_t proximity_warning = getProximityWarning();
	uint8_t high_temp = getTempWarning();

	sprintf(dest + strlen(dest), "Unsafe driving:    %d\n", unsafe_driving);
	//HAL_UART_Transmit(&huart2, (uint8_t*) g_tx_buffer, MESSAGE_LENGTH, MAX_TRANSMISSION);

	sprintf(dest + strlen(dest), "Impact detected:   %d\n", impact);
	//HAL_UART_Transmit(&huart2, (uint8_t*) g_tx_buffer, MESSAGE_LENGTH, MAX_TRANSMISSION);

	sprintf(dest + strlen(dest), "Low-Light warning: %d\n", low_light_warning);
	//HAL_UART_Transmit(&huart2, (uint8_t*) g_tx_buffer, MESSAGE_LENGTH, MAX_TRANSMISSION);

	sprintf(dest + strlen(dest), "Proximity warning: %d\n", proximity_warning);
	//HAL_UART_Transmit(&huart2, (uint8_t*) g_tx_buffer, MESSAGE_LENGTH, MAX_TRANSMISSION);

	sprintf(dest + strlen(dest), "High Temperature:  %d\n", high_temp);
	//HAL_UART_Transmit(&huart2, (uint8_t*) g_tx_buffer, MESSAGE_LENGTH, MAX_TRANSMISSION);
}

void displayTemp(char *dest) {

	float temp = getTemp();

	uint32_t t_whole;
	uint32_t t_decimal;
	char t_sign = sign(temp);

	WholeFraction(temp, 1, &t_whole, &t_decimal);

	sprintf(dest + strlen(dest), "Temperature: %c%02lu.%1lu C\n", t_sign,
			t_whole, t_decimal);
	//HAL_UART_Transmit(&huart2, (uint8_t*) g_tx_buffer, MESSAGE_LENGTH, 100);
}

void displayDistance(char *dest) {

	float distance = getDistance();

	uint32_t d_whole;
	uint32_t d_decimal;

	WholeFraction(distance, 1, &d_whole, &d_decimal);

	sprintf(dest + strlen(dest), "Distance:    %02lu.%1lu cm\n", d_whole,
			d_decimal);
	//HAL_UART_Transmit(&huart2, (uint8_t*) g_tx_buffer, MESSAGE_LENGTH, 100);
}

void displayLight(char *dest) {

	uint32_t light = getLight();

	sprintf(dest + strlen(dest), "Light:      %04lu lux\n", light);
	//HAL_UART_Transmit(&huart2, (uint8_t*) g_tx_buffer, MESSAGE_LENGTH, 100); //22 to include newline
}

void displayAccel(char *dest) {

	float x = getX();
	float y = getY();
	float z = getZ();

	char x_sign = sign(x);
	char y_sign = sign(y);
	char z_sign = sign(z);

	uint32_t x_whole, y_whole, z_whole;
	uint32_t x_decimal, y_decimal, z_decimal;

	WholeFraction(x, 2, &x_whole, &x_decimal);
	WholeFraction(y, 2, &y_whole, &y_decimal);
	WholeFraction(z, 2, &z_whole, &z_decimal);

	sprintf(dest + strlen(dest), "X accel:     %c%lu.%02lu g\n", x_sign,
			x_whole, x_decimal);
	//HAL_UART_Transmit(&huart2, (uint8_t*) g_tx_buffer, MESSAGE_LENGTH, 100);

	sprintf(dest + strlen(dest), "Y accel:     %c%lu.%02lu g\n", y_sign,
			y_whole, y_decimal);
	//HAL_UART_Transmit(&huart2, (uint8_t*) g_tx_buffer, MESSAGE_LENGTH, 100);

	sprintf(dest + strlen(dest), "X accel:     %c%lu.%02lu g\n", z_sign,
			z_whole, z_decimal);
	//HAL_UART_Transmit(&huart2, (uint8_t*) g_tx_buffer, MESSAGE_LENGTH, 100);
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

void checkAlarms() {

	//uint8_t unsafe_driving = getUnsafeDriving(); //accelerometer
	//uint8_t impact = getImpact();                //accelerometer
	//uint8_t low_light_warning = getLowLight();           //photodiode
	uint8_t proximity_warning = getProximityWarning();   //Distance Sensor
	uint8_t high_temp = getTempWarning();  //Temp sensor

	if (proximity_warning) {
		flashLED(D2);
	} else if (*LEDs[D2] == LED_FLASHING) {
		toggleLED(D2);
	}
	if (high_temp) {
		flashLED(D5);
	} else if (*LEDs[D5] == LED_FLASHING) {
		toggleLED(D5);
	}

}

void disableAlarms() {
	disableDistanceAlarmCheck();
	disableTempAlarmCheck();

	for (uint8_t led_index = 0; led_index < 4; led_index++) {
		if (*LEDs[led_index] == LED_FLASHING) {
			toggleLED(led_index);
			toggleLED(led_index);
			//*LEDs[led_index] = LED_OFF;
			//Toggling twice leaves the LED in the state it was on
		}
	}

}

void enableAlarms() {
	enableDistanceAlarmCheck();
	enableTempAlarmCheck();
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {

	//BUTTONS

	if (GPIO_Pin == MIDDLE_BUTTON_Pin) {
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

	//KEYPAD

	else if (GPIO_Pin == ROW_0_Pin) { //R
		rowDetected[0] = 1;
		rowTick[0] = HAL_GetTick();
	} else if (GPIO_Pin == ROW_1_Pin) { //R
		rowDetected[1] = 1;
		rowTick[1] = HAL_GetTick();
	} else if (GPIO_Pin == ROW_2_Pin) { //R
		rowDetected[2] = 1;
		rowTick[2] = HAL_GetTick();
	} else if (GPIO_Pin == ROW_3_Pin) { //R
		rowDetected[3] = 1;
		rowTick[3] = HAL_GetTick();
	}
}

void handleButton(ButtonIndex btn) {
	switch (btn) {

	case MIDDLE:
		if (HAL_GPIO_ReadPin(GPIOB, MIDDLE_BUTTON_Pin) == 1) {
			enableAlarms();
			*LEDs[D2] = LED_OFF;
			*LEDs[D3] = LED_OFF;
			*LEDs[D4] = LED_OFF;
			*LEDs[D5] = LED_OFF;
		}
		break;
	case UP:
		if (HAL_GPIO_ReadPin(GPIOB, UP_BUTTON_Pin) == 1) {
			toggleLED(D2);
			disableAlarms();
			//HAL_GPIO_TogglePin(GPIOA, D2_Pin);

		}
		break;
	case DOWN:
		if (HAL_GPIO_ReadPin(GPIOB, DOWN_BUTTON_Pin) == 1) {
			toggleLED(D5);
			disableAlarms();
		}
		break;
	case LEFT:
		if (HAL_GPIO_ReadPin(GPIOA, LEFT_BUTTON_Pin) == 1) {
			toggleLED(D3);
			disableAlarms();
		}
		break;
	case RIGHT:
		if (HAL_GPIO_ReadPin(GPIOA, RIGHT_BUTTON_Pin) == 1) {
			toggleLED(D4);
			disableAlarms();
		}
		break;

	}
	triggerDetected[btn] = 0;
}

void toggleLED(LEDIndex led) {

	switch (led) {

	case D2:
		if (HAL_GPIO_ReadPin(GPIOB, D2_Pin)) {
			*LEDs[D2] = LED_OFF;
		} else {
			*LEDs[D2] = LED_ON;
		}
		break;
	case D3:
		if (HAL_GPIO_ReadPin(GPIOA, D3_Pin)) {
			*LEDs[D3] = LED_OFF;
		} else {
			*LEDs[D3] = LED_ON;
		}

		break;
	case D4:
		if (HAL_GPIO_ReadPin(GPIOA, D4_Pin)) {
			*LEDs[D4] = LED_OFF;
		} else {
			*LEDs[D4] = LED_ON;
		}

		break;
	case D5:
		if (HAL_GPIO_ReadPin(GPIOA, D5_Pin)) {
			*LEDs[D5] = LED_OFF;
		} else {
			*LEDs[D5] = LED_ON;
		}

		break;

	}

}

void flashLED(LEDIndex led) {
	switch (led) {

	case D2:
		*LEDs[D2] = LED_FLASHING;
		break;
	case D3:
		*LEDs[D3] = LED_FLASHING;
		break;
	case D4:
		*LEDs[D4] = LED_FLASHING;
		break;
	case D5:
		*LEDs[D5] = LED_FLASHING;
		break;

	}
}

void scanButtons() {
	for (int i = 0; i < NUM_BUTTONS; i++) {
		if (triggerDetected[i]
				&& (HAL_GetTick() - triggerTick[i] > DEBOUNCE_TIME)) {
			handleButton(i);
		}
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART2) {

		if (rx_byte == START_CHAR) {
			//HAL_GPIO_TogglePin(GPIOA, D2_Pin);

			transmitting_message = 1;
			command_index = 0;

		} else if ((rx_byte == '\n' && !message_ready)
				|| (message_ready && rx_byte != '\n')) {
			command_index = 0;
			transmitting_message = 0;
			message_ready = 0;
		}

		else if (rx_byte == END_CHAR && transmitting_message) {
			command_str[command_index] = '\0';
			transmitting_message = 0;
			message_ready = 1;
		} else if (rx_byte == '\n' && message_ready) {
			command_ready = 1;
			message_ready = 0;
		} else if (transmitting_message) {

			if (command_index < (sizeof(command_str) - 1)) {
				command_str[command_index] = rx_byte;
				command_index += 1;
			} else {
				transmitting_message = 0;
				command_index = 0;
				command_ready = 0;
				message_ready = 0;
			}

		}

		// CRITICAL: You must call this again to listen for the NEXT byte
		HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
	}
}

void defaultSetup() {
	setAllCols(GPIO_PIN_SET);
	disableAlarms();

	HAL_UART_Receive_IT(&huart2, &rx_byte, 1);

	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);

	*LEDs[D2] = LED_ON;
	*LEDs[D3] = LED_ON;
	*LEDs[D4] = LED_ON;
	*LEDs[D5] = LED_ON;

}
