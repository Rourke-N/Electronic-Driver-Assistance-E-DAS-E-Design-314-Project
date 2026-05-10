#include "Keypad.h"
#include "General.h"

//PB2 = COL_0
//PA11 =  COL_1
//PC4 = COL_2

#define ROW_NUM 4
#define COL_NUM 3
#define KEY_BOUNCE 50

extern uint32_t rowTick[4];
extern uint8_t rowDetected[4];

extern UART_HandleTypeDef huart2;

char key_selected[4][3] = { { '1', '2', '3' }, { '4', '5', '6' }, { '7', '8',
		'9' }, { '*', '0', '#' } };

//static char key_buffer[40];
static int8_t activeRow = -1;

void handleKey(char key) {

	//snprintf(key_buffer, sizeof(key_buffer), "Key %c pressed\r\n", key);

	//HAL_UART_Transmit_IT(&huart2, (uint8_t*) key_buffer, strlen(key_buffer));

	UI_handleKey(key);

}

void setCol(uint8_t i, uint8_t state) { //i from 0 to 2

	switch (i) {
	case 0:
		HAL_GPIO_WritePin(GPIOB, COL_0_Pin, state);
		break;
	case 1:
		HAL_GPIO_WritePin(GPIOA, COL_1_Pin, state);
		break;
	case 2:
		HAL_GPIO_WritePin(GPIOC, COL_2_Pin, state);
		break;
	default:
		break;

	}
}

void setAllCols(uint8_t state) {
	setCol(0, state);
	setCol(1, state);
	setCol(2, state);
}

uint8_t ReadRow(uint8_t i) { //i from 0 to 2

	switch (i) {
	case 0:
		return (HAL_GPIO_ReadPin(GPIOB, ROW_0_Pin) == GPIO_PIN_SET);
		break;
	case 1:
		return (HAL_GPIO_ReadPin(GPIOB, ROW_1_Pin) == GPIO_PIN_SET);
		break;
	case 2:
		return (HAL_GPIO_ReadPin(GPIOB, ROW_2_Pin) == GPIO_PIN_SET);
		break;
	case 3:
		return (HAL_GPIO_ReadPin(GPIOB, ROW_3_Pin) == GPIO_PIN_SET);
		break;
	default:
		return 0;
		break;

	}
}

void scanKeys() {

	if (activeRow != -1) { //If a row was active check it
		setAllCols(GPIO_PIN_RESET);
		uint8_t stillDown = 0;

		// Check if ANY column is still making a connection for the active row
		for (uint8_t col = 0; col < COL_NUM; col++) {
			setCol(col, GPIO_PIN_SET);

			if (ReadRow(activeRow)) {
				stillDown = 1;
				setCol(col, GPIO_PIN_RESET);
				break;
			}
			setCol(col, GPIO_PIN_RESET);
		}

		if (!stillDown) {
			activeRow = -1;
			setAllCols(GPIO_PIN_SET);
			__HAL_GPIO_EXTI_CLEAR_IT(
					ROW_0_Pin | ROW_1_Pin | ROW_2_Pin | ROW_3_Pin);
		}
		return;
	}

	for (uint8_t row = 0; row < ROW_NUM; row++) {
        //Check if a row is high
		if (rowDetected[row] && (HAL_GetTick() - rowTick[row] > KEY_BOUNCE)) {
			setAllCols(GPIO_PIN_RESET);
			//Check columns one at a time to find out which one is cooking
			for (uint8_t col = 0; col < COL_NUM; col++) { //
				setCol(col, GPIO_PIN_SET);
				if (ReadRow(row)) {
					char key = key_selected[row][col];
					handleKey(key);
					activeRow = row;
					rowDetected[row] = 0;
					break;
				}
				setCol(col, GPIO_PIN_RESET);
			}
			if (activeRow != -1) break;
			setAllCols(GPIO_PIN_SET);
			rowDetected[row] = 0;
			}
		if (ReadRow(row)) {
			break;
		}
	}
}

