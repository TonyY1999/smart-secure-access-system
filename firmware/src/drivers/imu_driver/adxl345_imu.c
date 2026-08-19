/**************************************************************************//**
 * @file      adxl345.c
 * @brief     Template for ESE516 with Doxygen-style comments
 * @author    Tony Yan & Yue Zhang
 * @date      2025-04-07
 ******************************************************************************/

/******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdint.h>
#include <string.h>
#include "SerialConsole.h"
#include "I2cDriver.h"
#include "adxl345_imu.h"
#include "buzzer_driver/Buzzer.h"
#include "servo_driver/servo_driver.h"

/******************************************************************************
 * Variables
 ******************************************************************************/
uint8_t msgOutImu[64]; ///<USE ME AS A BUFFER FOR platform_write and platform_read
I2C_Data imuData; ///<as a structure to communicate with the IMU on platform_write and platform_read

extern TaskHandle_t wifiTaskHandle;

/******************************************************************************
 * Global Functions
 ******************************************************************************/
/**
 * @brief Write a single register to ADXL345
 */
int adxl_write(uint8_t reg, uint8_t val)
{
    msgOutImu[0] = reg;
    msgOutImu[1] = val;

    imuData.address = ADXL345_I2C_ADDR;
    imuData.msgOut = msgOutImu;
    imuData.lenOut = 2;
    imuData.msgIn = NULL;
    imuData.lenIn = 0;

    return I2cWriteDataWait(&imuData, pdMS_TO_TICKS(100));
}

/**
 * @brief Read one or more registers from ADXL345
 */
int adxl_read(uint8_t reg, uint8_t *buf, uint8_t len)
{
    msgOutImu[0] = reg;

    imuData.address = ADXL345_I2C_ADDR;
    imuData.msgOut = msgOutImu;
    imuData.lenOut = 1;
    imuData.msgIn = buf;
    imuData.lenIn = len;
	
	int32_t res = I2cReadDataWait(&imuData, pdMS_TO_TICKS(10), pdMS_TO_TICKS(100));
    //return I2cReadDataWait(&imuData, pdMS_TO_TICKS(10), pdMS_TO_TICKS(100));
	return res;
}

/**
 * @brief Initialize ADXL345 (check ID and enable measurement mode)
 */
//void adxl_init(void)
//{
    //uint8_t id = 0;
	////int res = adxl_read(ADXL345_REG_DEVID, &id, 1);
    ////if (adxl_read(ADXL345_REG_DEVID, &id, 1) != 0 || id != 0xE5) {
        ////SerialConsoleWriteString("[ADXL345] Device not found or ID mismatch\r\n");
        ////return -1;
    ////}
	////
	//while(adxl_read(ADXL345_REG_DEVID, &id, 1) != 0 || id != 0xE5) {
		//SerialConsoleWriteString("111");
		//vTaskDelay(pdMS_TO_TICKS(100));
	//}
	//
    //// Enable measurement mode
    ////if (adxl_write(ADXL345_REG_POWER_CTL, 0x08) != 0) {
        ////SerialConsoleWriteString("[ADXL345] Failed to set POWER_CTL\r\n");
        ////return -2;
    ////}
	//
	//while(adxl_write(ADXL345_REG_POWER_CTL, 0x08) != 0) {
		//vTaskDelay(pdMS_TO_TICKS(100));
	//}
	//
    //SerialConsoleWriteString("[ADXL345] Initialization complete\r\n");
//}

/**
 * @brief Initialize ADXL345 (check ID and enable measurement mode)
 */
int adxl_init(void)
{
	//vTaskSuspend(wifiTaskHandle);
    uint8_t id = 0;
	//int res = adxl_read(ADXL345_REG_DEVID, &id, 1);
    if (adxl_read(ADXL345_REG_DEVID, &id, 1) != 0 || id != 0xE5) {
        SerialConsoleWriteString("[ADXL345] Device not found or ID mismatch\r\n");
        return -1;
    }

    // Enable measurement mode
    if (adxl_write(ADXL345_REG_POWER_CTL, 0x08) != 0) {
        SerialConsoleWriteString("[ADXL345] Failed to set POWER_CTL\r\n");
        return -2;
    }

    SerialConsoleWriteString("[ADXL345] Initialization complete\r\n");
	//vTaskResume(wifiTaskHandle);
    return 0;
}


/**
 * @brief Read X, Y, Z acceleration (raw values)
 */
int adxl_read_xyz(int16_t *x, int16_t *y, int16_t *z)
{
    uint8_t data[6];
    if (adxl_read(ADXL345_REG_DATAX0, data, 6) != 0) {
        return -1;
    }
	
	vTaskDelay(pdMS_TO_TICKS(10));

    *x = (int16_t)(data[1] << 8 | data[0]);
    *y = (int16_t)(data[3] << 8 | data[2]);
    *z = (int16_t)(data[5] << 8 | data[4]);
	
    return 0;
}

#define VIBRATION_THRESHOLD 300
extern TaskHandle_t fingerTaskHandle;

void vIMUTask(void *pvParameters)
{
	// init IMU + buzzer
	adxl_init();
	buzzer_init();
	
	int16_t x, y, z;
	int16_t prev_x = 0, prev_y = 0, prev_z = 0;
	bool first_sample = true;
	
	while (1) {				
		if (adxl_read_xyz(&x, &y, &z) == 0)
		{
			if (first_sample) {
				prev_x = x;
				prev_y = y;
				prev_z = z;
				first_sample = false;
			}
			
			char msg[64];
			snprintf(msg, sizeof(msg), "Accel X: %d, Y: %d, Z: %d\r\n", x, y, z);
			SerialConsoleWriteString(msg);

			if (abs(x - prev_x) > VIBRATION_THRESHOLD || abs(y - prev_y) > VIBRATION_THRESHOLD || abs(z - prev_z) > VIBRATION_THRESHOLD) {
				SerialConsoleWriteString("!!! VIBRATION DETECTED !!! Triggering buzzer!\r\n");

				port_pin_set_output_level(LED_0_PIN,LED_0_ACTIVE);
				buzzer_init();
				buzzer_on();
			}

			prev_x = x;
			prev_y = y;
			prev_z = z;
			
			vTaskDelay(pdMS_TO_TICKS(50));
		}
		else {
			SerialConsoleWriteString("Error reading ADXL345 data!\r\n");
		}

		vTaskDelay(pdMS_TO_TICKS(15000));
	}
}