#include "Accelerometer.h"
#include "General.h"

#define I2C_ADDRESS 0x68
#define PWR_MGMT_1_REG 0x6B
#define WHO_AM_I_REG 0x75
#define SMPLRT_DIV_REG 0x19
#define ACCEL_CONFIG_REG 0x1C
#define ACCEL_XOUT_REG 0x3B
#define INT_ENABLE_REG 0x38
#define INT_STATUS_REG 0x3A

const uint32_t MPU6050_ADDR = I2C_ADDRESS << 1;

extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart2;

float x_accel = 0;
float y_accel = 0;
float z_accel = 0;

int16_t Accel_X_RAW;
int16_t Accel_Y_RAW;
int16_t Accel_Z_RAW;

const float MAX_ACCEL = 9.99;
const float MIN_ACCEL = -9.99;

const float CONVERSION = 1.0f / 16384.0f;

uint8_t MPU_OK = 0;

char msgbuffer[] = "Testing MPU6050";

volatile uint8_t raw_data[6] = { 0 };
uint8_t flag = 0;

void MPU6050_Init() {
	uint8_t check = 0; // Initialize to 0 so you know if it changed
	uint8_t Data = 0;
	//char debugBuffer[50]; // Buffer to hold the string
	//HAL_UART_Transmit(&huart2, (uint8_t*) msgbuffer, strlen(msgbuffer),100);
	// Perform the read
	HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR, WHO_AM_I_REG, 1, &check, 1, 1000);
	//sprintf(debugBuffer, "\r\nWHO_AM_I returned: 0x%02X\r\n", check);

	// Send it to UART
	//HAL_UART_Transmit(&huart2, (uint8_t*) debugBuffer, strlen(debugBuffer),100);

	if (check == 0x68) {
		//char okbuffer[] = "MPU6050 OK\n";
		//HAL_UART_Transmit(&huart2, (uint8_t*) okbuffer, strlen(okbuffer), 100);
		MPU_OK = 1;
	} else if (check == 0x98) { //ICM-20689 is actually the accelerometer I am using
		//char ICM[] = "ICM-20689 found!\n";
		MPU_OK = 1;
		//HAL_UART_Transmit(&huart2, (uint8_t*) ICM, strlen(ICM), 100);
	} else {
		char error[] = "None found!\n";
		HAL_UART_Transmit(&huart2, (uint8_t*) error, strlen(error), 100);
	}

	if (MPU_OK) {

		//Reset All registers in PWR management to 0
		Data = 0;
		HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, PWR_MGMT_1_REG, 1, &Data, 1,
				1000);

		//Set Sample Rate
		//SAMPLE_RATE = INTERNAL_SAMPLE_RATE / (1 + SMPLRT_DIV)
		/*
		 Divides the internal sample rate (see register CONFIG) to generate the sample
		 rate that controls sensor data output rate, FIFO sample rate.
		 Note: This register is only effective when FCHOICE_B register bits are 2’b00, and
		 (0 < DLPF_CFG < 7).
		 This is the update rate of the sensor register:
		 SAMPLE_RATE = INTERNAL_SAMPLE_RATE / (1 + SMPLRT_DIV)
		 Where INTERNAL_SAMPLE_RATE = 1kHz
		 */

		Data = 0;
		HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, SMPLRT_DIV_REG, 1, &Data, 1,
				1000);
		// Set accelerometer configuration in ACCEL_CONFIG_REG Register
		Data = 0x00; // XA_ST=0,YA_ST=0,ZA_ST=0, FS_SEL=0 -> <strong>±</strong> 2g
		HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, ACCEL_CONFIG_REG, 1, &Data, 1,
				1000);

		//Enable the data ready interrupt
		Data = 0x01;
		HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, INT_ENABLE_REG, 1, &Data, 1,
				1000);
	}

}

void updateData() {

	Accel_X_RAW = (int16_t) ((uint16_t) raw_data[0] << 8
			| (uint16_t) raw_data[1]);
	Accel_Y_RAW = (int16_t) ((uint16_t) raw_data[2] << 8
			| (uint16_t) raw_data[3]);
	Accel_Z_RAW = (int16_t) ((uint16_t) raw_data[4] << 8
			| (uint16_t) raw_data[5]);

	x_accel = (float) Accel_Y_RAW * CONVERSION; //My X is Y
	y_accel = (-1.0f) * (float) Accel_X_RAW * CONVERSION; //My Y is -X
	z_accel = (float) Accel_Z_RAW * CONVERSION;

	if (x_accel >= MAX_ACCEL) {
		x_accel = MAX_ACCEL;
	} else if (x_accel <= MIN_ACCEL) {
		x_accel = MIN_ACCEL;
	}

	if (y_accel >= MAX_ACCEL) {
		y_accel = MAX_ACCEL;
	} else if (y_accel <= MIN_ACCEL) {
		y_accel = MIN_ACCEL;
	}

	if (z_accel >= MAX_ACCEL) {
		z_accel = MAX_ACCEL;
	} else if (z_accel <= MIN_ACCEL) {
		z_accel = MIN_ACCEL;
	}
}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c) {
	updateData();
}

void clearIntFlag() {
	HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR, INT_STATUS_REG, 1, &flag, 1,10); //Reads data and clears interrupt flag
}

void readAccel() {
	HAL_I2C_Mem_Read_DMA(&hi2c1, MPU6050_ADDR, ACCEL_XOUT_REG, 1,
			(uint8_t*) raw_data, 6); //Reads data
}
uint8_t getMPU_OK() {
	return MPU_OK;
}

float getX() {
	return x_accel;
}
float getY() {
	return y_accel;
}
float getZ() {
	return z_accel;
}
uint8_t getUnsafeDriving() {
	return 0;
}

uint8_t getImpactWarning() {
	return 0;
}

void clearUnsafeWarning(uint8_t delay) {

}
void clearImpactWarning(uint8_t delay) {

}
void str_Accel_OLED(char *dest, size_t size) {

	float magnitude = sqrtf(
			x_accel * x_accel + y_accel * y_accel + z_accel * z_accel);

	uint32_t whole;
	uint32_t decimal;

	WholeFraction(magnitude, 2, &whole, &decimal);

	snprintf(dest, size, "Accel:    |%lu.%02lu| g\n", whole, decimal);
}

void str_Accel_UART(char *dest, size_t size) {

	float x = x_accel;
	float y = y_accel;
	float z = z_accel;

	char x_sign = sign(x);
	char y_sign = sign(y);
	char z_sign = sign(z);

	uint32_t x_whole, y_whole, z_whole;
	uint32_t x_decimal, y_decimal, z_decimal;

	WholeFraction(x, 2, &x_whole, &x_decimal);
	WholeFraction(y, 2, &y_whole, &y_decimal);
	WholeFraction(z, 2, &z_whole, &z_decimal);

	// Using snprintf with pointer arithmetic requires updating the remaining size
	int len = 0;

	len += snprintf(dest + len, size - len, "X accel:     %c%lu.%02lu g\n",
			x_sign, x_whole, x_decimal);

	len += snprintf(dest + len, size - len, "Y accel:     %c%lu.%02lu g\n",
			y_sign, y_whole, y_decimal);

	snprintf(dest + len, size - len, "Z accel:     %c%lu.%02lu g\n", z_sign,
			z_whole, z_decimal);
}

