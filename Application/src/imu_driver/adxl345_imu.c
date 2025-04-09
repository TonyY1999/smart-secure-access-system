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

/******************************************************************************
 * Defines
 ******************************************************************************/
#define ADXL345_I2C_ADDR		0x53  // Default address
#define ADXL345_REG_DEVID		0x00
#define ADXL345_REG_POWER_CTL	0x2D
#define ADXL345_REG_DATAX0		0x32

/******************************************************************************
 * Variables
 ******************************************************************************/
uint8_t msgOutImu[64]; ///<USE ME AS A BUFFER FOR platform_write and platform_read
I2C_Data imuData; ///<Use me as a structure to communicate with the IMU on platform_write and platform_read

/******************************************************************************
 * Forward Declarations
 ******************************************************************************/

/******************************************************************************
 * Callback Functions
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

    return I2cReadDataWait(&imuData, pdMS_TO_TICKS(10), pdMS_TO_TICKS(100));
}

/**
 * @brief Initialize ADXL345 (check ID and enable measurement mode)
 */
int adxl_init(void)
{
    uint8_t id = 0;
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
