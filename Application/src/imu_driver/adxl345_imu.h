/**************************************************************************//**
 * @file      adxl345.h
 * @brief     Template for ESE516 with Doxygen-style comments
 * @author    Tony Yan & Yue Zhang
 * @date      2025-04-0
 ******************************************************************************/

#ifndef ADXL345_H
#define ADXL345_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Defines
 ******************************************************************************/
#define ADXL345_I2C_ADDR		0x53  // Default address
#define ADXL345_REG_DEVID		0x00
#define ADXL345_REG_POWER_CTL	0x2D
#define ADXL345_REG_DATAX0		0x32

/******************************************************************************
 * Global Function Declaration
 ******************************************************************************/
/**
 * @brief Write a value to a register on the ADXL345 over I2C.
 *
 * @param[in] reg Register address to write to.
 * @param[in] val Value to write into the register.
 * @return 0 if successful, negative value on failure.
 */
int adxl_write(uint8_t reg, uint8_t val);

/**
 * @brief Read one or more bytes from a register on the ADXL345.
 *
 * @param[in]  reg  Register address to start reading from.
 * @param[out] buf  Buffer to store the read data.
 * @param[in]  len  Number of bytes to read.
 * @return 0 if successful, negative value on failure.
 */
int adxl_read(uint8_t reg, uint8_t *buf, uint8_t len);

/**
 * @brief Initialize the ADXL345 accelerometer.
 *
 * Checks the device ID and sets measurement mode.
 *
 * @return 0 if successful, negative value if device ID mismatched or init failed.
 */
void adxl_init();

/**
 * @brief Read raw X, Y, and Z acceleration data from the ADXL345.
 *
 * @param[out] x Pointer to store raw X-axis data.
 * @param[out] y Pointer to store raw Y-axis data.
 * @param[out] z Pointer to store raw Z-axis data.
 * @return 0 if successful, negative value on failure.
 */
int adxl_read_xyz(int16_t *x, int16_t *y, int16_t *z);
void vIMUTask(void *pvParameters);

#ifdef __cplusplus
}
#endif

#endif  // ADXL345_H