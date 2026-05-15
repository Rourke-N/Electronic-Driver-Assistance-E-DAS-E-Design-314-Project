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

uint32_t alarm_tick = 0;
uint8_t pre_unsafeWarn = 0;
#define ACCEL_WAIT 400


const float MAX_ACCEL = 9.99;
const float MIN_ACCEL = -9.99;

float magnitude = 0;

const float CONVERSION = 1.0f / 16384.0f;

volatile uint8_t raw_data[6] = { 0 };
uint8_t flag = 0;

uint8_t MPU_OK = 0;

char msgbuffer[] = "Testing MPU6050";

uint8_t vectored_Accel = 0;

#define AVG_SAMPLES 20

typedef struct {
	int16_t x[AVG_SAMPLES];
	int16_t y[AVG_SAMPLES];
	int16_t z[AVG_SAMPLES];
	uint8_t index;
} RawBuffer;

RawBuffer calibrationBuffer = { 0 };

int16_t raw_gravityX = 0;
int16_t raw_gravityY = 0;
int16_t raw_gravityZ = 0;

float gravityX = 0;
float gravityY = 0;
float gravityZ = 0;

uint8_t sampleCount = 0;

uint8_t unsafeDriving_flag = 0;
uint8_t impact_flag = 0;

// --- Recalculated Calibration Coefficients (May 10) ---
static const float m_x = 0.000061185f;
static const float c_x = -0.01126f;

static const float m_y = 0.000060968f;
static const float c_y = 0.00281f;

static const float m_z = 0.000060763f;
static const float c_z = -0.014735f;
// -

// Call this every time you get a new sensor reading (e.g., in your DMA callback)
void updateRawBuffer(int16_t rawX, int16_t rawY, int16_t rawZ) {
	calibrationBuffer.x[calibrationBuffer.index] = rawX;
	calibrationBuffer.y[calibrationBuffer.index] = rawY;
	calibrationBuffer.z[calibrationBuffer.index] = rawZ;

	calibrationBuffer.index = (calibrationBuffer.index + 1) % AVG_SAMPLES;
}

void getAverageRaw(int16_t *avgX, int16_t *avgY, int16_t *avgZ) {
	int32_t sumX = 0, sumY = 0, sumZ = 0;

	for (int i = 0; i < AVG_SAMPLES; i++) {
		sumX += calibrationBuffer.x[i];
		sumY += calibrationBuffer.y[i];
		sumZ += calibrationBuffer.z[i];
	}

	*avgX = (int16_t) (sumX / AVG_SAMPLES);
	*avgY = (int16_t) (sumY / AVG_SAMPLES);
	*avgZ = (int16_t) (sumZ / AVG_SAMPLES);
}

void updateData() {

	Accel_X_RAW = (int16_t) ((uint16_t) raw_data[0] << 8
			| (uint16_t) raw_data[1]);
	Accel_Y_RAW = (int16_t) ((uint16_t) raw_data[2] << 8
			| (uint16_t) raw_data[3]);
	Accel_Z_RAW = (int16_t) ((uint16_t) raw_data[4] << 8
			| (uint16_t) raw_data[5]);

	x_accel = ((float) Accel_Y_RAW * m_y) + c_y - gravityX; //My X is Y
	y_accel = ((-1.0f) * (float) Accel_X_RAW * m_x) + c_x - gravityY; //My Y is -X
	z_accel = ((float) Accel_Z_RAW * m_z) + c_z - gravityZ;

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
	if (sampleCount < AVG_SAMPLES) {
		updateRawBuffer(Accel_X_RAW, Accel_Y_RAW, Accel_Z_RAW);
		sampleCount++;
	} else if (!vectored_Accel) {
		vectored_Accel = 1;
		getAverageRaw(&raw_gravityX, &raw_gravityY, &raw_gravityZ);

		gravityX = ((float) raw_gravityY * m_y) + c_y;
		gravityY = ((-1.0f) * (float) raw_gravityX * m_x) + c_x;
		gravityZ = ((float) raw_gravityZ * m_z) + c_z;

	} else {
		updateRawBuffer(Accel_X_RAW, Accel_Y_RAW, Accel_Z_RAW);
	}

	magnitude = sqrtf(
			x_accel * x_accel + y_accel * y_accel + (z_accel * z_accel));

	if (magnitude > MAX_ACCEL) {
		magnitude = MAX_ACCEL;
	}

	uint32_t interval = HAL_GetTick() - alarm_tick;

	//It must not update flag until that interval has passed
	//Only update flags after interval
/*
	if(magnitude > 0.50f){
		alarm_tick = HAL_GetTick();
		pre_unsafeWarn = 1;
	}
	if(magnitude > 1.50f){
			impact_flag = 1;
			pre_unsafeWarn = 0;
		}
	else if (pre_unsafeWarn && interval > ACCEL_WAIT) {
        unsafeDriving_flag = 1;
        pre_unsafeWarn = 0;
	}else{
		impact_flag = 0;
		unsafeDriving_flag = 0;
	}
*/

	if (magnitude > 1.50f) {
		impact_flag = 1;
		unsafeDriving_flag = 0;
	} else if (magnitude > 0.50f) {
		impact_flag = 0;
		unsafeDriving_flag = 1;
	} else {
		impact_flag = 0;
		unsafeDriving_flag = 0;
	}

}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c) {
	updateData();
}

void clearIntFlag() {
	HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR, INT_STATUS_REG, 1, &flag, 1, 10); //Reads data and clears interrupt flag
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

float getMag() {
	return magnitude;
}

uint8_t getUnsafeDriving() {
	return unsafeDriving_flag;
}

uint8_t getImpactWarning() {
	return impact_flag;
}

void MPU6050_Init_1() {
	uint8_t Data = 0x80;
	HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, PWR_MGMT_1_REG, 1, &Data, 1, 100);
	MPU_OK = 0;
// Returns immediately — main.c waits 100ms then calls Init_2
}

uint8_t checkAccelStatus(void) {
	uint8_t check = 0;
	HAL_StatusTypeDef status;

// Read the WHO_AM_I register (Timeout set to 10ms so it doesn't block long if disconnected)
	status = HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR, WHO_AM_I_REG, 1, &check, 1,
			10);

// If I2C communication succeeded AND it returned the correct ID (ICM-20689 or MPU6050)
	if (status == HAL_OK && (check == 0x68 || check == 0x98)) {
		MPU_OK = 1;
	} else {
		MPU_OK = 0;
	}

	return MPU_OK;
}

void MPU6050_Init_2_A() {
	uint8_t check = 0;
	uint8_t Data = 0;

	uint32_t start = HAL_GetTick();
	do {
		HAL_Delay(5);
		Data = 0xFF;
		HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR, PWR_MGMT_1_REG, 1, &Data, 1,
				100);
	} while ((Data & 0x80) && (HAL_GetTick() - start < 100));

// 2. Confirm chip is alive before doing anything else
	HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR, WHO_AM_I_REG, 1, &check, 1, 100);
	if (check != 0x68 && check != 0x98) {
		char error[] = "Accel Init Failed: No Device\n";
		HAL_UART_Transmit(&huart2, (uint8_t*) error, strlen(error), 100);
		MPU_OK = 0;
		return;
	}
	MPU_OK = 1;

// 3. Reset signal paths
	Data = 0x07;
	HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, 0x68, 1, &Data, 1, 100);
	HAL_Delay(10);

// 1. Reset signal paths
	Data = 0x07;
	HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, 0x68, 1, &Data, 1, 100);
	HAL_Delay(10);

// 2. Verify identity
	HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR, WHO_AM_I_REG, 1, &check, 1, 100);
	if (check == 0x68 || check == 0x98) {
		MPU_OK = 1;
	} else {
		char error[] = "Accel Init Failed: No Device\n";
		HAL_UART_Transmit(&huart2, (uint8_t*) error, strlen(error), 100);
		return;
	}

// 3. Wake up, PLL clock source
	Data = 0x01;
	HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, PWR_MGMT_1_REG, 1, &Data, 1, 100);

// 4. Gyro FCHOICE_B = 0b00 — this is the key fix for 32kHz
//    Register 0x1B (GYRO_CONFIG), bits [1:0] = FCHOICE_B
//    Must be 0b00 for DLPF to be active, otherwise bypasses to 32kHz
	Data = 0x00;
	HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, 0x1B, 1, &Data, 1, 100);

// 5. DLPF_CFG in CONFIG register (0x1A)
//    With FCHOICE_B = 0b00 and DLPF_CFG = 3:
//    Gyro BW = 41Hz, internal sample rate = 1kHz
	Data = 0x03;
	HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, 0x1A, 1, &Data, 1, 100);

// 6. ACCEL_CONFIG_2 (0x1D) — ICM-20689 specific
//    Bits [3:2] = ACCEL_FCHOICE_B must be 0b00
//    Bits [1:0] = A_DLPF_CFG = 3 (44.8Hz BW, 1kHz internal rate)
//    0x03 = ACCEL_FCHOICE_B:0b00, A_DLPF_CFG:0b11
	Data = 0x03;
	HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, 0x1D, 1, &Data, 1, 100);

// 7. Sample rate divider
//    SAMPLE_RATE = 1kHz / (1 + 1) = 500Hz
//    Both FCHOICE_B paths now active so this register is respected
	Data = 0x01;
	HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, SMPLRT_DIV_REG, 1, &Data, 1, 100);

// 8. Accel config: ±2g
	Data = 0x00;
	HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, ACCEL_CONFIG_REG, 1, &Data, 1, 100);

// 9. INT pin: pulse mode, active high, push-pull
	Data = 0x00;
	HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, 0x37, 1, &Data, 1, 100);

// 10. Enable data-ready interrupt
	Data = 0x01;
	HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, INT_ENABLE_REG, 1, &Data, 1, 100);

// 11. Clear any startup pending interrupt before EXTI fires
	HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR, INT_STATUS_REG, 1, &Data, 1, 100);
}

void clearUnsafeWarning(void) {

}
void clearImpactWarning() {

}
void str_Accel_OLED(char *dest, size_t size) {

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

// FIX: start from current end of buffer, not from 0
	size_t len = strlen(dest);

	if (len < size) {
		len += snprintf(dest + len, size - len, "X accel:     %c%lu.%02lu g\n",
				x_sign, x_whole, x_decimal);
	}
	if (len < size) {
		len += snprintf(dest + len, size - len, "Y accel:     %c%lu.%02lu g\n",
				y_sign, y_whole, y_decimal);
	}
	if (len < size) {
		snprintf(dest + len, size - len, "Z accel:     %c%lu.%02lu g\n", z_sign,
				z_whole, z_decimal);
	}
}

// Format: ±x.xx,±x.xx,±x.xx  (X,Y,Z — two decimal places each)
// Returns all three axes as a single comma-separated string
void str_Accel_SD(char *dest, size_t size) {
	float x = getX();
	float y = getY();
	float z = getZ();

	char xs = (x < 0) ? '-' : ' ';
	char ys = (y < 0) ? '-' : ' ';
	char zs = (z < 0) ? '-' : ' ';

	uint32_t xw, xd, yw, yd, zw, zd;
	WholeFraction(x, 2, &xw, &xd);
	WholeFraction(y, 2, &yw, &yd);
	WholeFraction(z, 2, &zw, &zd);

// Strip leading space for positive values to match PDD example format
	int len = 0;
	if (xs == ' ') {
		len += snprintf(dest + len, size - len, "%lu.%02lu,", xw, xd);
	} else {
		len += snprintf(dest + len, size - len, "-%lu.%02lu,", xw, xd);
	}
	if (ys == ' ') {
		len += snprintf(dest + len, size - len, "%lu.%02lu,", yw, yd);
	} else {
		len += snprintf(dest + len, size - len, "-%lu.%02lu,", yw, yd);
	}
	if (zs == ' ') {
		snprintf(dest + len, size - len, "%lu.%02lu", zw, zd);
	} else {
		snprintf(dest + len, size - len, "-%lu.%02lu", zw, zd);
	}
}
