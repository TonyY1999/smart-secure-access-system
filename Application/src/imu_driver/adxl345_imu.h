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
 * Includes
 ******************************************************************************/
#include "I2cDriver/I2cDriver.h"

/******************************************************************************
 * Defines
 ******************************************************************************/

/******************************************************************************
 * Structures and Enumerations
 ******************************************************************************/

/******************************************************************************
 * Global Function Declaration
 ******************************************************************************/
int adxl_write(uint8_t reg, uint8_t val);
int adxl_read(uint8_t reg, uint8_t *buf, uint8_t len);
int adxl_init(void);
int adxl_read_xyz(int16_t *x, int16_t *y, int16_t *z);
/**
 * @fn		int ExampleFuncionComment(int inputInt, void *pvParameters)
 * @brief	An example function comment. Erase me!
 * @details 	Write details of function here.

 * @param[in]	InputInt Use me to describe inputs to functions
 * @param[out]	*pvParameters Use me to describe outputs of functions passed as arguments
 * @return		Use me to explain the return of an argument.
 * @note
 */



#ifdef __cplusplus
}
#endif

#endif  // ADXL345_H