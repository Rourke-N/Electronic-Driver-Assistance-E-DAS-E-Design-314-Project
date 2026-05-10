#include "General.h"

//Date
#define YEAR 2026
uint8_t month = 2;
uint8_t day = 26;
uint8_t hour = 22;
uint8_t minute = 12;
uint8_t second = 42;

//LEDS
volatile uint32_t *LEDs[] = { &TIM3->CCR4, // D2
		&TIM3->CCR1, // D3
		&TIM3->CCR2, // D4
		&TIM1->CCR2  // D5
		};

//BUTTONS

volatile uint32_t triggerTick[6] = { 0 };
volatile uint8_t triggerDetected[6] = { 0 };
#define DEBOUNCE_TIME 50

//KEYPAD
volatile uint32_t rowTick[4] = { 0 };
volatile uint8_t rowDetected[4] = { 0 };

//UART/
const char START_CHAR = '@';
const char END_CHAR = '&';
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart6;

extern volatile uint8_t rx_data;

volatile uint8_t rx_byte;
volatile char command_str[50];
volatile uint8_t command_index = 0;

volatile uint8_t transmitting_message = 0;
volatile uint8_t message_ready = 0;
volatile uint8_t command_ready;

char g_tx_buffer[150];
#define MESSAGE_LENGTH 21
#define MAX_TRANSMISSION 100
static char display_buffer[500];

//Timers
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim3;

//OLED
#define REFRESH_TIME 40 //20ms 50 frames per second
MenuElement_t *currentMenu = &Display_main;
uint32_t last_refresh = 0;

uint8_t editing_fuel = 0;
uint8_t editing_km = 0;
float old_fuel;
float new_fuel = 0;
float old_distance_ODO;
float new_distance_ODO = 0;

//Accel
uint32_t last_accel_read = 0;
#define ACCEL_TIMEOUT 500
uint8_t accel_int_flag = 0;

//Alarms
#define NO_ALARM -1
#define NUM_ALARMS 5
uint8_t lastSet[5] = { -1, -1, -1, -1, -1 }; //position 0 is last set
uint8_t numSet = 0;

//Light
extern TIM_HandleTypeDef htim5;

typedef enum {
	UNSAFE_WARN, IMPACT_WARN, LIGHT_WARN, PROX_WARN, TEMP_WARN
} AlarmType;

MenuElement_t *warningMenu[5] = { &Warn_UnsafeDriving, &Warn_Impact,
		&Warn_Light, &Warn_Proximity, &Warn_Temp };

uint8_t (*getWarning[])(void) = {
	getUnsafeDriving,
	getImpactWarning,
	getLightWarning,
	getProximityWarning,
	getTempWarning
};

void (*clear_alarm[])(uint8_t) = {
	clearUnsafeWarning,
	clearImpactWarning,
	clearLightWarning,
	clearProximityWarning,
	clearTempWarning
};

uint8_t enableCheck[5] = { 1, 1, 1, 1, 1 };

uint8_t isAlarmActive(AlarmType alarm) {
	for (uint8_t i = 0; i < numSet; i++) {
		if (lastSet[i] == alarm)
			return 1;
	}
	return 0;
}

void pushAlarm(AlarmType alarm) {

	switch (alarm) {
	case PROX_WARN:
		flashLED(D2);
		break;
	case IMPACT_WARN:
		*LEDs[D3] = LED_ON;
		break;
	case UNSAFE_WARN:
		flashLED(D3);
		break;
	case LIGHT_WARN:
		flashLED(D4);
		break;
	case TEMP_WARN:
		flashLED(D5);
		break;
	}

	if (numSet >= NUM_ALARMS)
		return;

	for (uint8_t i = numSet; i > 0; i--) {
		lastSet[i] = lastSet[i - 1];
	}
	lastSet[0] = alarm;
	numSet += 1;

}

void removeAlarm(AlarmType alarm) {

	switch (alarm) {
	case PROX_WARN:
		*LEDs[D2] = LED_OFF;
		break;
	case IMPACT_WARN:
		*LEDs[D3] = LED_OFF;
		break;
	case UNSAFE_WARN:
		*LEDs[D3] = LED_OFF;
		break;
	case LIGHT_WARN:
		*LEDs[D4] = LED_OFF;
		break;
	case TEMP_WARN:
		*LEDs[D5] = LED_OFF;
		break;
	}

	if (numSet == 0)
		return;

	uint8_t position = 0;
	uint8_t found = 0;

	for (uint8_t ind = 0; ind < NUM_ALARMS; ind++) {
		if (lastSet[ind] == alarm) {
			position = ind;
			found = 1;
		}
	}

	if (!found) {
		return;
	}

	for (uint8_t i = position; i < numSet - 1; i++) {
		lastSet[i] = lastSet[i + 1];
	}

	numSet -= 1;
	lastSet[numSet] = NO_ALARM;

}

//if an alarm is already set then dont check it

//Only clear the alarm normally if it wasnt set

void checkAlarms() //Checking real alarms
//If an alarm is set by setWarn, then disable checking of alarms automatically
{

	for (int i = 0; i < NUM_ALARMS; i++) {
		if (enableCheck[i]) {
			if (getWarning[i]() == 1 && !isAlarmActive(i)) {

				//flashLED(D2);

				pushAlarm(i);
			} else if (getWarning[i]() == 0 && isAlarmActive(i)) { //No alarm
				removeAlarm(i);
				enableCheck[i] = 1;
			}
		}
	}
}

void mainLoop() {
	if (command_ready) {
		handleCommand();
		command_ready = 0;
	}

	sampleTempSensor();
	sampleDistanceSensor();
	//checkAlarms();
	scanButtons();
	scanKeys();
	UI_Refresh();

	if (HAL_GetTick() - last_accel_read > ACCEL_TIMEOUT) {
		//readAccel(); // Force a read to reset the INT pin
		last_accel_read = HAL_GetTick();
		clearIntFlag();

	}  if (accel_int_flag) {
		clearIntFlag();
		readAccel();
		last_accel_read = HAL_GetTick();
		accel_int_flag = 0;
	}

}

void handleCommand() {

	display_buffer[0] = '\0';

	if (strcmp((char*) command_str, "Stat") == 0) {
		// Clear the buffer first to start fresh
		display_buffer[0] = '\0';

		// Append the start character
		snprintf(display_buffer, sizeof(display_buffer), "%c", START_CHAR);

		// Pass the buffer AND its capacity to the functions
		str_Date_UART(display_buffer, sizeof(display_buffer));
		str_dist_UART(display_buffer, sizeof(display_buffer));
		str_temp_UART(display_buffer, sizeof(display_buffer));
		str_LUX_UART(display_buffer, sizeof(display_buffer));
		str_Accel_UART(display_buffer, sizeof(display_buffer));
		str_AlarmConditions_UART(display_buffer, sizeof(display_buffer));
		str_GPS_UART(display_buffer, sizeof(display_buffer));

		// Final end character
		size_t len = strlen(display_buffer);
		snprintf(display_buffer + len, sizeof(display_buffer) - len, "%c\n",
				END_CHAR);
		HAL_UART_Transmit_IT(&huart2, (uint8_t*) display_buffer,
				strlen(display_buffer));

	} else if (strncmp((char*) command_str, "SetWarn", 7) == 0) {
		uint8_t alarm = command_str[8] - '0' - 1; //-1 to adjust to my enum
		uint8_t value = command_str[10] - '0';
		if (value == 1 && alarm < 5 && alarm >= 0) {
			removeAlarm(alarm);
			pushAlarm(alarm);
			enableCheck[alarm] = 0;
		}
		if (value == 0) {
			if (numSet > 0 && alarm < 5 && alarm >= 0) { // Safer than checking != NO_ALARM
				removeAlarm(alarm);
				enableCheck[alarm] = 1;
				clear_alarm[alarm](0);
			}
		}
	}

	else if (strcmp((char*) command_str, "Log") == 0) {
		setLogging(!getLogging());
	} else if (strncmp((char*) command_str, "SFD", 3) == 0) {
		float fuel = 0;
		float km = 0;
		float multiplier = 100.0f;
		for (uint8_t i = 4; i <= 8; i++) {
			char digit = command_str[i];
			if (digit >= '0' && digit <= '9') {
				fuel += multiplier * ((digit) - '0');
				multiplier = multiplier / 10;
			}
		}
		multiplier = 100.0f;
		for (uint8_t i = 10; i <= 14; i++) {
			char digit = command_str[i];
			if (digit >= '0' && digit <= '9') {
				km += multiplier * ((digit) - '0');
				multiplier = multiplier / 10;
			}
		}

		setFuel(fuel);
		setDistance_ODO(km);

	} else if (strcmp((char*) command_str, "RFE") == 0) {
		display_buffer[0] = '\0';
		size_t len = 0;
		len += snprintf(display_buffer, sizeof(display_buffer), "%c",
				START_CHAR);
		str_Date_UART(display_buffer, sizeof(display_buffer));
		str_FuelEfficiency_UART(display_buffer, sizeof(display_buffer));
		len = strlen(display_buffer);
		snprintf(display_buffer + len, sizeof(display_buffer) - len, "%c\n",
				END_CHAR);
		HAL_UART_Transmit_IT(&huart2, (uint8_t*) display_buffer,
				strlen(display_buffer));

	} else if (strcmp((char*) command_str, "Disable Light") == 0) {
		removeAlarm(LIGHT_WARN);
		enableCheck[LIGHT_WARN] = 0;
	} else if (strcmp((char*) command_str, "Disable Temp") == 0) {
		removeAlarm(TEMP_WARN);
		enableCheck[TEMP_WARN] = 0;
	}

}

void str_Date_UART(char *dest, size_t space) {
	if (space) {
		sprintf(dest + strlen(dest), "%04u/%02u/%02u %02u:%02u:%02u \n",
		YEAR, month, day, hour, minute, second);
	} else {
		sprintf(dest + strlen(dest), "%04u/%02u/%02u %02u:%02u:%02u\n",
		YEAR, month, day, hour, minute, second);
	}
//HAL_UART_Transmit(&huart2, (uint8_t*) g_tx_buffer, MESSAGE_LENGTH, MAX_TRANSMISSION);
}
void str_AlarmConditions_UART(char *dest, size_t size) {
	size_t len = strlen(dest);

	// Line 1: 19 (label) + 1 (val) + 1 (\n) = 21 chars
	if (len < size) {
		len += snprintf(dest + len, size - len, "Unsafe driving:    %d\n",
				isAlarmActive(UNSAFE_WARN));
	}

	// Line 2
	if (len < size) {
		len += snprintf(dest + len, size - len, "Impact detected:   %d\n",
				isAlarmActive(IMPACT_WARN));
	}

	// Line 3
	if (len < size) {
		len += snprintf(dest + len, size - len, "Low-Light warning: %d\n",
				isAlarmActive(LIGHT_WARN));
	}

	// Line 4
	if (len < size) {
		len += snprintf(dest + len, size - len, "Proximity warning: %d\n",
				isAlarmActive(PROX_WARN));
	}

	// Line 5
	if (len < size) {
		snprintf(dest + len, size - len, "High Temperature:  %d\n",
				isAlarmActive(TEMP_WARN));
	}
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
	if (value >= 0) {
		return ' ';
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

void enableAlarms() {
//enableDistanceAlarmCheck();
//enableTempAlarmCheck();
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
	} else if (GPIO_Pin == ACCEL_INT_Pin) {
		accel_int_flag = 1;
	}
}

void handleButton(ButtonIndex btn) {
	switch (btn) {

	case MIDDLE:
		if (HAL_GPIO_ReadPin(GPIOB, MIDDLE_BUTTON_Pin) == 1) {
			if (numSet > 0) {			// Safer than checking != NO_ALARM
				AlarmType activeAlarm = lastSet[0];
				if (activeAlarm < NUM_ALARMS) {
					removeAlarm(activeAlarm);
					enableCheck[activeAlarm] = 1;
					clear_alarm[activeAlarm](0);
				}
			} else {

				if (currentMenu == &Data_1 && !editing_fuel) {
					editing_fuel = 1;
					new_fuel = 0;
					old_fuel = getFuel();
					str_toggleS3(editing_fuel);
				} else if (currentMenu == &Data_1 && editing_fuel) {
					editing_fuel = 0;
					str_toggleS3(editing_fuel);
				} else if (currentMenu == &Data_2 && !editing_km) {
					editing_km = 1;
					new_distance_ODO = 0;
					old_distance_ODO = getDistance_ODO();
					str_toggleS3(editing_km);
				} else if (currentMenu == &Data_2 && editing_km) {
					editing_km = 0;
					str_toggleS3(editing_km);
				}

			}
		}
		break;
	case UP:
		if (HAL_GPIO_ReadPin(GPIOB, UP_BUTTON_Pin) == 1) {
			//toggleLED(D2);
			if (numSet == 0) {

				//if (currentMenu->up != NULL)
				//currentMenu = currentMenu->up;
				if (currentMenu->down != NULL)
					currentMenu = currentMenu->down;
				if (editing_fuel) {
					editing_fuel = 0;
					setFuel(old_fuel);
					editing_km = editing_km;
					str_toggleS3(0);
				} else if (editing_km) {
					editing_km = 0;
					editing_km = 0;
					setDistance_ODO(old_distance_ODO);
					str_toggleS3(editing_km);
				}
			}
		}

		break;
	case DOWN:
		if (HAL_GPIO_ReadPin(GPIOB, DOWN_BUTTON_Pin) == 1) {
			if (numSet == 0) {
				//if (currentMenu->down != NULL)
				//currentMenu = currentMenu->down;
				if (currentMenu->up != NULL)
					currentMenu = currentMenu->up;
				if (editing_fuel) {
					editing_fuel = 0;
					setFuel(old_fuel);
					editing_km = editing_km;
					str_toggleS3(0);
				} else if (editing_km) {
					editing_km = 0;
					editing_km = 0;
					setDistance_ODO(old_distance_ODO);
					str_toggleS3(editing_km);
				}
			}
		}
		break;
	case LEFT:
		if (HAL_GPIO_ReadPin(GPIOA, LEFT_BUTTON_Pin) == 1) {
			//toggleLED(D3);
			if (numSet == 0) {

				if (currentMenu == &Data_1 && editing_fuel) {
					editing_fuel = 0;
					new_fuel = 0;
					setFuel(old_fuel);
					str_toggleS3(0);
				} else if (currentMenu == &Data_2 && editing_km) {
					editing_km = 0;
					new_distance_ODO = 0;
					setDistance_ODO(old_distance_ODO);
					str_toggleS3(0);
				}
				if (currentMenu->parent != NULL)
					currentMenu = currentMenu->parent;
			}
		}

		break;
	case RIGHT:
		if (HAL_GPIO_ReadPin(GPIOA, RIGHT_BUTTON_Pin) == 1) {
			//toggleLED(D4);
			//disableAlarmChecks();
			if (numSet == 0) {
				if (currentMenu->child != NULL)
					currentMenu = currentMenu->child;
			}
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
		HAL_UART_Receive_IT(&huart2, (uint8_t*) &rx_byte, 1);
	} else if (huart->Instance == USART6) {
		//HAL_UART_Transmit(&huart2, (uint32_t*) &rx_data, 1, 10); // Blocking send
		//GPS_UART_CallBack();
	}

}

void UI_handleKey(char key) {

	if (editing_fuel && key != '#' && key != '*' && new_fuel < 100) {
		new_fuel *= 10;
		new_fuel = new_fuel + 0.1 * (float) (key - '0');
		setFuel(new_fuel);
	} else if (editing_km && key != '#' && key != '*'
			&& new_distance_ODO < 100) {
		new_distance_ODO *= 10;
		new_distance_ODO = new_distance_ODO + 0.1 * (float) (key - '0');
		setDistance_ODO(new_distance_ODO);
	} else if (currentMenu == &Data_3) {
		if (key == '*') {
			setLogging(1);
			//str_toggleLOG(1);
		} else if (key == '#') {
			setLogging(0);
			//str_toggleLOG(0);
		}

	}
}

void UI_Refresh() {

	if (HAL_GetTick() - last_refresh > REFRESH_TIME) {
		last_refresh = HAL_GetTick();
		if (numSet > 0) {				// Safer than checking != NO_ALARM
			AlarmType activeAlarm = lastSet[0];
			if (activeAlarm < NUM_ALARMS) {
				if (warningMenu[activeAlarm] != NULL
						&& warningMenu[activeAlarm]->render != NULL) {
					warningMenu[activeAlarm]->render();

				}
			}

		} else if (currentMenu->render != NULL) {
			currentMenu->render();
		}
	}
}

void defaultSetup() {
	setAllCols(GPIO_PIN_SET);
//disableAlarmChecks();
	HAL_UART_Receive_IT(&huart2, (uint8_t*) &rx_byte, 1);
	HAL_UART_Receive_IT(&huart6, (uint8_t*) &rx_data, 1);
	HAL_TIM_OC_Start(&htim5, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);

	*LEDs[D2] = LED_OFF;
	*LEDs[D3] = LED_OFF;
	*LEDs[D4] = LED_OFF;
	*LEDs[D5] = LED_OFF;

//OLED
	Menu_Init();
	init_OLED();
	init_Light_Sensor();
	UI_Refresh();
	GPS_Init();
	MPU6050_Init();

}
