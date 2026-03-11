/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>   // Required for sprintf
#include <string.h>  // Required for strlen
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

//Buttons
typedef enum {
	MIDDLE, UP, DOWN, LEFT, RIGHT
} ButtonIndex;

uint8_t D3_ON = 1;
uint8_t D5_ON = 1;

volatile int triggerTick[6] = { 0 };
volatile uint8_t triggerDetected[6] = { 0 }; //
#define DEBOUNCE_TIME 35

//UART
uint8_t rx_byte;
char command_str[50];
uint8_t command_index = 0;
volatile uint8_t transmitting_message = 0;
volatile uint8_t command_ready = 0;
char tx_buffer[50];
uint8_t message_ready = 0;

//TEMP SENSOR
#define T_SAMPLE_SIZE 5
#define PULSE_TRAIN_LENGTH 60
const float T_CONVERT = 256.0f / 4096.0f;

uint32_t first_pulse_tick = 0;
uint32_t pulse_count;
uint8_t counting_temp = 0;

float real_temp_value;

float current_tempC;

float t_samples[T_SAMPLE_SIZE] = { 0 };
uint8_t t_sample_index = 0;

//DISTANCE SENSOR
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

#define D_INTERVAL 250

float current_distance;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM4_Init(void);
/* USER CODE BEGIN PFP */
void handleButton(ButtonIndex btn);
void defaultSetup();
void sampleTempSensor();
void sampleDistanceSensor();
void displayTempSamples();
void displayDistance();
void handleCommand();
char sign(float value);
void WholeFraction(float value, uint8_t precision, uint32_t *whole,
		uint8_t *decimal);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_USART2_UART_Init();
	MX_TIM1_Init();
	MX_TIM3_Init();
	MX_TIM2_Init();
	MX_TIM4_Init();
	/* USER CODE BEGIN 2 */

	defaultSetup(); // Set up
	//my_tim2_setup();
	HAL_TIM_Base_Start(&htim2);
	HAL_TIM_IC_Start_IT(&htim4, TIM_CHANNEL_2);
	HAL_UART_Receive_IT(&huart2, &rx_byte, 1); // Listen for 1 byte in the background

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	uint8_t sent = 0;
	uint32_t boot_time = HAL_GetTick();

	while (1) {

		if (!sent && (HAL_GetTick() - boot_time) > 100) {
			sprintf(tx_buffer, "*27547957#\n");
			HAL_UART_Transmit(&huart2, (uint8_t*) tx_buffer, strlen(tx_buffer),
					1000);
			sent = 1;
		}

		if (command_ready) {
			handleCommand();
			command_ready = 0;
		}

		sampleTempSensor();
		sampleDistanceSensor();

		for (int i = 0; i < 5; i++) {
			if (triggerDetected[i]
					&& (HAL_GetTick() - triggerTick[i] > DEBOUNCE_TIME)) {
				handleButton(i);
			}
		}

		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = 16;
	RCC_OscInitStruct.PLL.PLLN = 336;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
	RCC_OscInitStruct.PLL.PLLQ = 4;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief TIM1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM1_Init(void) {

	/* USER CODE BEGIN TIM1_Init 0 */

	/* USER CODE END TIM1_Init 0 */

	TIM_MasterConfigTypeDef sMasterConfig = { 0 };
	TIM_OC_InitTypeDef sConfigOC = { 0 };
	TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = { 0 };

	/* USER CODE BEGIN TIM1_Init 1 */

	/* USER CODE END TIM1_Init 1 */
	htim1.Instance = TIM1;
	htim1.Init.Prescaler = 41999;
	htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim1.Init.Period = 999;
	htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim1.Init.RepetitionCounter = 0;
	htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_PWM_Init(&htim1) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig)
			!= HAL_OK) {
		Error_Handler();
	}
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 500;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
	sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
	if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2)
			!= HAL_OK) {
		Error_Handler();
	}
	__HAL_TIM_DISABLE_OCxPRELOAD(&htim1, TIM_CHANNEL_2);
	sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
	sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
	sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
	sBreakDeadTimeConfig.DeadTime = 0;
	sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
	sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
	sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
	if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig)
			!= HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM1_Init 2 */

	/* USER CODE END TIM1_Init 2 */
	HAL_TIM_MspPostInit(&htim1);

}

/**
 * @brief TIM2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM2_Init(void) {

	/* USER CODE BEGIN TIM2_Init 0 */

	/* USER CODE END TIM2_Init 0 */

	TIM_SlaveConfigTypeDef sSlaveConfig = { 0 };
	TIM_MasterConfigTypeDef sMasterConfig = { 0 };

	/* USER CODE BEGIN TIM2_Init 1 */

	/* USER CODE END TIM2_Init 1 */
	htim2.Instance = TIM2;
	htim2.Init.Prescaler = 0;
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim2.Init.Period = 4294967295;
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim2) != HAL_OK) {
		Error_Handler();
	}
	sSlaveConfig.SlaveMode = TIM_SLAVEMODE_EXTERNAL1;
	sSlaveConfig.InputTrigger = TIM_TS_ETRF;
	sSlaveConfig.TriggerPolarity = TIM_TRIGGERPOLARITY_NONINVERTED;
	sSlaveConfig.TriggerPrescaler = TIM_TRIGGERPRESCALER_DIV1;
	sSlaveConfig.TriggerFilter = 8;
	if (HAL_TIM_SlaveConfigSynchro(&htim2, &sSlaveConfig) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig)
			!= HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM2_Init 2 */

	/* USER CODE END TIM2_Init 2 */

}

/**
 * @brief TIM3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM3_Init(void) {

	/* USER CODE BEGIN TIM3_Init 0 */

	/* USER CODE END TIM3_Init 0 */

	TIM_MasterConfigTypeDef sMasterConfig = { 0 };
	TIM_OC_InitTypeDef sConfigOC = { 0 };

	/* USER CODE BEGIN TIM3_Init 1 */

	/* USER CODE END TIM3_Init 1 */
	htim3.Instance = TIM3;
	htim3.Init.Prescaler = 42000;
	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim3.Init.Period = 999;
	htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_PWM_Init(&htim3) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig)
			!= HAL_OK) {
		Error_Handler();
	}
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 500;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1)
			!= HAL_OK) {
		Error_Handler();
	}
	__HAL_TIM_DISABLE_OCxPRELOAD(&htim3, TIM_CHANNEL_1);
	/* USER CODE BEGIN TIM3_Init 2 */

	/* USER CODE END TIM3_Init 2 */
	HAL_TIM_MspPostInit(&htim3);

}

/**
 * @brief TIM4 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM4_Init(void) {

	/* USER CODE BEGIN TIM4_Init 0 */

	/* USER CODE END TIM4_Init 0 */

	TIM_MasterConfigTypeDef sMasterConfig = { 0 };
	TIM_IC_InitTypeDef sConfigIC = { 0 };

	/* USER CODE BEGIN TIM4_Init 1 */

	/* USER CODE END TIM4_Init 1 */
	htim4.Instance = TIM4;
	htim4.Init.Prescaler = 83;
	htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim4.Init.Period = 65535;
	htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_IC_Init(&htim4) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig)
			!= HAL_OK) {
		Error_Handler();
	}
	sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_BOTHEDGE;
	sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
	sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
	sConfigIC.ICFilter = 0;
	if (HAL_TIM_IC_ConfigChannel(&htim4, &sConfigIC, TIM_CHANNEL_2) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM4_Init 2 */

	/* USER CODE END TIM4_Init 2 */

}

/**
 * @brief USART2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART2_UART_Init(void) {

	/* USER CODE BEGIN USART2_Init 0 */

	/* USER CODE END USART2_Init 0 */

	/* USER CODE BEGIN USART2_Init 1 */

	/* USER CODE END USART2_Init 1 */
	huart2.Instance = USART2;
	huart2.Init.BaudRate = 57600;
	huart2.Init.WordLength = UART_WORDLENGTH_9B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_EVEN;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart2) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN USART2_Init 2 */

	/* USER CODE END USART2_Init 2 */

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	/* USER CODE BEGIN MX_GPIO_Init_1 */

	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOA, D2_Pin | D4_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin : BUTTON_Pin */
	GPIO_InitStruct.Pin = BUTTON_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(BUTTON_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : D2_Pin D4_Pin */
	GPIO_InitStruct.Pin = D2_Pin | D4_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pins : LEFT_BUTTON_Pin RIGHT_BUTTON_Pin */
	GPIO_InitStruct.Pin = LEFT_BUTTON_Pin | RIGHT_BUTTON_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pin : TRIG_Pin */
	GPIO_InitStruct.Pin = TRIG_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(TRIG_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : MIDDLE_BUTTON_Pin UP_BUTTON_Pin DOWN_BUTTON_Pin */
	GPIO_InitStruct.Pin = MIDDLE_BUTTON_Pin | UP_BUTTON_Pin | DOWN_BUTTON_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/* EXTI interrupt init*/
	HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI3_IRQn);

	HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI4_IRQn);

	HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

	HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

	/* USER CODE BEGIN MX_GPIO_Init_2 */

	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE BEGIN 4 */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART2) {
		if (rx_byte == '@') {
			HAL_GPIO_TogglePin(GPIOA, D2_Pin);

			transmitting_message = 1;
			command_index = 0;
		} else if (rx_byte == '\n' && !message_ready) {
			command_index = 0;
			transmitting_message = 0;
		}

		else if (rx_byte == '&' && transmitting_message) {
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

char sign(float value) {
	if (value > 0) {
		return '+';
	} else {
		return '-';
	}
}

void displayTemp() {

	uint32_t t_whole;
	uint8_t t_decimal;
	char t_sign = sign(real_temp_value);

	WholeFraction(real_temp_value, 1, &t_whole, &t_decimal);

	sprintf(tx_buffer, "Temperature:  %c%02lu.%1u C\n", t_sign, t_whole,
			t_decimal);
	HAL_UART_Transmit(&huart2, (uint8_t*) tx_buffer, 22, 100);
}

void handleCommand() {
	if (strcmp(command_str, "Stat") == 0) {

		displayDistance();
		displayTemp();
	}
}

void sampleTempSensor() {

	pulse_count = TIM2->CNT;

	if (pulse_count > 0 && !counting_temp) {
		first_pulse_tick = HAL_GetTick();
		counting_temp = 1;
	}
	if (counting_temp
			&& (HAL_GetTick() - first_pulse_tick >= PULSE_TRAIN_LENGTH)) {

		current_tempC = (pulse_count * T_CONVERT) - 50.0f;

		t_samples[t_sample_index] = current_tempC;

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

		real_temp_value = (total - min - max) / (T_SAMPLE_SIZE - 2.0f);
		t_sample_index = 0;
	}
}

void WholeFraction(float value, uint8_t precision, uint32_t *whole,
		uint8_t *decimal) {
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

void displayDistance() {
	uint32_t d_whole;
	uint8_t d_decimal;

	WholeFraction(current_distance, 1, &d_whole, &d_decimal);

	sprintf(tx_buffer, "Distance:     %02lu.%1u cm\n", d_whole, d_decimal);
	HAL_UART_Transmit(&huart2, (uint8_t*) tx_buffer, 22, 100); //22 to include newline
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

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {

	if (htim->Instance == TIM4) {
		if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7) == GPIO_PIN_SET) {
			start_count = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
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
//void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {

	if (GPIO_Pin == MIDDLE_BUTTON_Pin || GPIO_Pin == BUTTON_Pin) {
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
		if (HAL_GPIO_ReadPin(GPIOB, MIDDLE_BUTTON_Pin) == 1
				|| (HAL_GPIO_ReadPin(GPIOC, BUTTON_Pin))) {

			displayTemp();
			displayDistance();

			//sprintf(tx_buffer, "Middle Button pressed\r\n");
			//HAL_UART_Transmit(&huart2, (uint8_t*) tx_buffer, strlen(tx_buffer), 1000);
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

void defaultSetup() {

	HAL_GPIO_WritePin(GPIOA, D2_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, D4_Pin, GPIO_PIN_SET);

	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);

}

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
	/* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
