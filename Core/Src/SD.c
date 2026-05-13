#include "SD.h"


char str_km_l[12];
char str_l_100km[15];

char str_fuel[12];

float fuel;
float distance;


uint8_t log_data = 0;

uint8_t SD_OK = 0;
const float MAX_EFF = 99.9;

extern UART_HandleTypeDef huart2;

#define LOG_FILENAME "log.csv"

FATFS FatFs; 	//Fatfs handle
FIL fil; 		//File handle
FRESULT fres; //Result after operations
DWORD free_clusters, free_sectors, total_sectors;
FATFS *getFreeFs;
BYTE readBuf[500];

FRESULT SD_Mount(void) {
    extern SPI_HandleTypeDef hspi2;  // adjust to your SPI handle name

    fres = f_mount(&FatFs, "", 1);
    if (fres != FR_OK) {
        // Reset SPI peripheral to clear any stuck bus state
        // This is what allows recovery after card removal/reinsertion
        HAL_SPI_DeInit(&hspi2);
        HAL_Delay(10);
        HAL_SPI_Init(&hspi2);

        myprintf("SD Mount Error: (%i)\r\n", fres);
        SD_OK = 0;
    } else {
        SD_OK = 1;
    }
    return fres;
}

FRESULT SD_Read(const char *filename, char *outBuffer, uint16_t bufferSize) {
	fres = f_open(&fil, filename, FA_READ);
	if (fres != FR_OK) {
		myprintf("SD Read: Failed to open %s (%i)\r\n", filename, fres);
		return fres;
	}

	if (f_gets(outBuffer, bufferSize, &fil) != NULL) {
		myprintf("SD Read: Success from %s\r\n", filename);
	} else {
		myprintf("SD Read: File empty or error\r\n");
	}

	f_close(&fil);
	return fres;
}

FRESULT SD_Write(const char *filename, const char *data) {
	UINT bytesWrote;

	// Open for writing (Create always overwrites existing)
	fres = f_open(&fil, filename, FA_WRITE | FA_OPEN_ALWAYS | FA_CREATE_ALWAYS);
	if (fres != FR_OK) {
		myprintf("SD Write: Failed to open %s (%i)\r\n", filename, fres);
		return fres;
	}

	fres = f_write(&fil, data, strlen(data), &bytesWrote);
	if (fres != FR_OK) {
		myprintf("SD Write: Failed to write to %s (%i)\r\n", filename, fres);
	} else {
		myprintf("SD Write: Wrote %i bytes to %s\r\n", bytesWrote, filename);
	}

	f_close(&fil); // Always close!
	return fres;
}

void SD_test() {

	myprintf("\r\n~ SD card demo by kiwih ~\r\n\r\n");

	HAL_Delay(1000); //a short delay is important to let the SD card settle

	//some variables for FatFs
	FATFS FatFs; 	//Fatfs handle
	FIL fil; 		//File handle
	FRESULT fres; //Result after operations

	//Open the file system
	fres = f_mount(&FatFs, "", 1); //1=mount now
	if (fres != FR_OK) {
		myprintf("f_mount error (%i)\r\n", fres);
		while (1)
			;
	}

	//Let's get some statistics from the SD card
	DWORD free_clusters, free_sectors, total_sectors;

	FATFS *getFreeFs;

	fres = f_getfree("", &free_clusters, &getFreeFs);
	if (fres != FR_OK) {
		myprintf("f_getfree error (%i)\r\n", fres);
		while (1)
			;
	}

	//Formula comes from ChaN's documentation
	total_sectors = (getFreeFs->n_fatent - 2) * getFreeFs->csize;
	free_sectors = free_clusters * getFreeFs->csize;

	myprintf(
			"SD card stats:\r\n%10lu KiB total drive space.\r\n%10lu KiB available.\r\n",
			total_sectors / 2, free_sectors / 2);

	//Now let's try to open file "test.txt"
	fres = f_open(&fil, "test.txt", FA_READ);
	if (fres != FR_OK) {
		myprintf("f_open error (%i)\r\n", fres);
		while (1)
			;
	}
	myprintf("I was able to open 'test.txt' for reading!\r\n");

	//Read 30 bytes from "test.txt" on the SD card
	BYTE readBuf[30];

	//We can either use f_read OR f_gets to get data out of files
	//f_gets is a wrapper on f_read that does some string formatting for us
	TCHAR *rres = f_gets((TCHAR*) readBuf, 30, &fil);
	if (rres != 0) {
		myprintf("Read string from 'test.txt' contents: %s\r\n", readBuf);
	} else {
		myprintf("f_gets error (%i)\r\n", fres);
	}

	//Be a tidy kiwi - don't forget to close your file!
	f_close(&fil);

	//Now let's try and write a file "write.txt"
	fres = f_open(&fil, "write.txt",
	FA_WRITE | FA_OPEN_ALWAYS | FA_CREATE_ALWAYS);
	if (fres == FR_OK) {
		myprintf("I was able to open 'write.txt' for writing\r\n");
	} else {
		myprintf("f_open error (%i)\r\n", fres);
	}

	//Copy in a string
	strncpy((char*) readBuf, "a new file is made!", 20);
	UINT bytesWrote;
	fres = f_write(&fil, readBuf, 19, &bytesWrote);
	if (fres == FR_OK) {
		myprintf("Wrote %i bytes to 'write.txt'!\r\n", bytesWrote);
	} else {
		myprintf("f_write error (%i)\r\n", fres);
	}

	//Be a tidy kiwi - don't forget to close your file!
	f_close(&fil);

	//We're done, so de-mount the drive
	f_mount(NULL, "", 0);
}

void myprintf(const char *fmt, ...) {
	static char buffer[256];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);

	int len = strlen(buffer);
	HAL_UART_Transmit(&huart2, (uint8_t*) buffer, len, -1);

}

uint8_t getSD_OK() {
	return SD_OK;
}

void setLogging(uint8_t log) {
	log_data = log;
}

uint8_t getLogging() {
	return log_data;
}

void setDistance_ODO(float newdistance) {
	distance = newdistance;
}

float getDistance_ODO() {
	return distance;
}

void setFuel(float newfuel) {
	fuel = newfuel;
}

float getFuel() {
	return fuel;
}


void update_strs() {
	float km_l;
	if (fuel != 0) {
		km_l = distance / fuel;
		if (km_l > MAX_EFF) {
			km_l = MAX_EFF;
		}
	} else {
		km_l = 0;
	}

	float l_100km;
	if (distance != 0) {
		l_100km = 100.0f * (fuel / distance);
		if (l_100km > MAX_EFF) {
			l_100km = MAX_EFF;
		}
	} else {
		l_100km = 0;
	}

	uint32_t km_l_whole;
	uint32_t km_l_decimal;

	uint32_t l_100km_whole;
	uint32_t l_100km_decimal;

	WholeFraction(km_l, 1, &km_l_whole, &km_l_decimal);
	WholeFraction(l_100km, 1, &l_100km_whole, &l_100km_decimal);

	snprintf(str_km_l, sizeof(str_km_l), "%02lu.%01lu", km_l_whole,
			km_l_decimal);
	snprintf(str_l_100km, sizeof(str_l_100km), " %02lu.%01lu", l_100km_whole,
			l_100km_decimal);

}

void str_fuel_OLED(char *dest, size_t size) {

	uint32_t fuel_whole;
	uint32_t fuel_decimal;

	WholeFraction(fuel, 1, &fuel_whole, &fuel_decimal);

	snprintf(dest, size, "Current:   %03lu.%01lu L", fuel_whole, fuel_decimal);

}

void str_dist_ODO_OLED(char *dest, size_t size) {

	uint32_t dist_whole;
	uint32_t dist_decimal;

	WholeFraction(distance, 1, &dist_whole, &dist_decimal);

	snprintf(dest, size, "Current:  %03lu.%01lu km", dist_whole, dist_decimal);

}

void str_FuelEfficiency_UART(char *dest, size_t size) {

	update_strs();

	size_t len = strlen(dest);
	snprintf(dest + len, size - len, "Fuel Eff: %s km/L%s L/100km\n", str_km_l,
			str_l_100km);
}

// Change signature to void since OLED.c never uses the return value
void checkSD(void) {
    static uint32_t last_check_tick = 0;

    if (HAL_GetTick() - last_check_tick < 2000) {
        return;  // Now valid — void function
    }
    last_check_tick = HAL_GetTick();

    f_mount(NULL, "", 0);

    if (f_mount(&FatFs, "", 1) == FR_OK) {
        SD_OK = 1;
    } else {
        SD_OK = 0;
    }
}

void str_FuelEfficiency_OLED(char *dest1, char *dest2, size_t size) {

	update_strs();

	snprintf(dest1, size, "         %s km/L\n", str_km_l);

	snprintf(dest2, size, "    %s L/100 km\n", str_l_100km);
}

FRESULT SD_Clear_Log(void) {
    // 1. Mount the SD card
    if (SD_Mount() == FR_OK) {

        // 2. Opening with FA_CREATE_ALWAYS + FA_WRITE
        // wipes the file contents immediately.
        fres = f_open(&fil, LOG_FILENAME, FA_CREATE_ALWAYS | FA_WRITE);

        if (fres == FR_OK) {
           // myprintf("SD Log: %s cleared successfully.\r\n", LOG_FILENAME);
            f_close(&fil);
        } else {
            myprintf("SD Log: Failed to clear %s (%i)\r\n", LOG_FILENAME, fres);
        }

        // 3. Unmount to be safe
        f_mount(NULL, "", 0);
    }
    return fres;
}

// Deletes the log file from the SD card
FRESULT SD_Delete_Log(void) {
    // 1. Check if logging is currently active
    if (log_data == 1) {
        myprintf("SD Log: Cannot delete while logging is ENABLED\r\n");
        return FR_DENIED; // Return an error if logging is active
    }

    if (SD_Mount() == FR_OK) {
        // 2. f_unlink removes the file from the directory
        fres = f_unlink(LOG_FILENAME);

        if (fres == FR_OK) {
          //  myprintf("SD Log: %s deleted successfully.\r\n", LOG_FILENAME);
        } else if (fres == FR_NO_FILE) {
          //  myprintf("SD Log: File does not exist (nothing to delete).\r\n");
        } else {
            myprintf("SD Log: Delete failed (%i)\r\n", fres);
        }

        f_mount(NULL, "", 0);
    }
    return fres;
}

void SD_Dump(void) {

	char start = START_CHAR;

    // 1. Send the PDD Start Character
    HAL_UART_Transmit(&huart2, (uint8_t*)&start, 1, 100);

    // 2. Mount and Open
    if (SD_Mount() == FR_OK) {
        if (f_open(&fil, LOG_FILENAME, FA_READ) == FR_OK) {

            uint8_t chunk[512]; // Small, stack-safe buffer
            UINT bytesRead;

            // 3. Keep reading until f_read says there's nothing left
            do {
                fres = f_read(&fil, chunk, sizeof(chunk), &bytesRead);

                if (fres == FR_OK && bytesRead > 0) {
                    // Send the chunk we just pulled from the SD card
                    HAL_UART_Transmit(&huart2, chunk, bytesRead, 1000);
                }
            } while (fres == FR_OK && bytesRead > 0);

            f_close(&fil);
        } else {
            //myprintf("SD Dump: Failed to open %s\r\n", LOG_FILENAME);
        }
        // Unmount to flush everything
        f_mount(NULL, "", 0);
    }

    // 4. Send the PDD End Characters
    // Using a small local string for the terminator
    char end_seq[3] = {END_CHAR, '\n', '\0'};
    HAL_UART_Transmit(&huart2, (uint8_t*)end_seq, 2, 100);
}

// Master log function — assembles one full CSV line and appends
// ============================================================
void SD_Log_Data(void) {

	if (!log_data)
		return;  // Logging disabled — do nothing

	// Mount — your SD_Mount already handles error printing
	if (SD_Mount() != FR_OK)
		return;

	// Build the CSV line in a local buffer
	// Worst case line length from PDD Table 9:
	// "2026/04/29 17:06:00,0742,33.4,05.4,0.15,0.35,1.57,1,1,0,1,1,-71.673611,-002.828611\n"
	// = ~85 chars. 150 is safe.
	char line[150];
	line[0] = '\0';
	int len = 0;

	// Temporary field buffers
	char field[30];

	// 1. Date and time
	str_Date_SD(field, sizeof(field));
	len += snprintf(line + len, sizeof(line) - len, "%s,", field);

	// 2. Light (lux)
	str_Light_SD(field, sizeof(field));
	len += snprintf(line + len, sizeof(line) - len, "%s,", field);

	// 3. Temperature
	str_Temp_SD(field, sizeof(field));
	len += snprintf(line + len, sizeof(line) - len, "%s,", field);

	// 4. Distance
	str_Dist_SD(field, sizeof(field));
	len += snprintf(line + len, sizeof(line) - len, "%s,", field);
	// 5. X, Y, Z acceleration (comma-separated, function includes commas between axes)
	str_Accel_SD(field, sizeof(field));
	len += snprintf(line + len, sizeof(line) - len, "%s,", field);

	// 6. Warning flags (unsafe,impact,light,proximity,temp)
	str_Warnings_SD(field, sizeof(field));
	len += snprintf(line + len, sizeof(line) - len, "%s,", field);

	// 7. GPS lat,long
	str_GPS_SD(field, sizeof(field));
	len += snprintf(line + len, sizeof(line) - len, "%s\n", field);

	// Open file for APPEND — FA_OPEN_APPEND creates if not exists,
	// appends if it does. Do NOT use FA_CREATE_ALWAYS here or you
	// will overwrite the file every log cycle.
	UINT bytesWritten;
	fres = f_open(&fil, LOG_FILENAME, FA_WRITE | FA_OPEN_APPEND);
	if (fres != FR_OK) {
		myprintf("SD_Log_Data: f_open failed (%d)\r\n", fres);
		f_mount(NULL, "", 0);
		return;
	}

	fres = f_write(&fil, line, strlen(line), &bytesWritten);
	if (fres != FR_OK) {
		myprintf("SD_Log_Data: f_write failed (%d)\r\n", fres);
	}

	f_close(&fil);
	f_mount(NULL, "", 0);  // Unmount after each write to flush FAT
}
