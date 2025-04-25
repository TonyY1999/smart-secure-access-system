/**************************************************************************//**
 * @file      fingerprint_driver.h
 * @brief     fingerprint module driver
 * @author    Tony Yan & Yue Zhang
 * @date      2025-04-16
 ******************************************************************************/

#ifndef FINGERPRINT_DRIVER_H
#define FINGERPRINT_DRIVER_H 

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdint.h>

/******************************************************************************
 * Defines
 ******************************************************************************/
#define GEN_IMG_CMD			{0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x03, 0x01, 0x00, 0x05}
#define GEN_CF_TO_B1_CMD	{0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x04, 0x02, 0x01, 0x00, 0x08}
#define GEN_CF_TO_B2_CMD	{0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x04, 0x02, 0x02, 0x00, 0x09}
#define REG_MODEL_CMD		{0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x03, 0x05, 0x00, 0x09}
#define STORE_CMD(ID)		{0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x06, 0x06, 0x01, (uint8_t)(ID >> 8), (uint8_t)(ID & 0xFF), (uint8_t)((0x0E + ID) >> 8), (uint8_t)((0x0E + ID) & 0xFF)}
#define DELETE_CMD(ID)		{0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x07, 0x0C, (uint8_t)(ID >> 8), (uint8_t)(ID & 0xFF), 0x00, 0x01, (uint8_t)(0x15 + ID) >> 8, (uint8_t)(0x15 + ID) & 0xFF};
#define SEARCH_CMD			{0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x08, 0x04, 0x01, 0x00, 0x00, 0x00, 0x32, 0x00, 0x40}
#define READ_SYS_CMD		{0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x03, 0x0F, 0x00, 0x13}
#define TEMP_NUM_CMD		{0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x03, 0x1D, 0x00, 0x21}

/******************************************************************************
 * Structures and Enumerations
 ******************************************************************************/

/******************************************************************************
 * Global Function Declaration
 ******************************************************************************/
void fingerprint_init();

uint8_t gen_img();

uint8_t gen_cf_to_b1();

uint8_t gen_cf_to_b2();

uint8_t reg_model();

uint8_t store_finger(uint8_t id);

uint8_t fingerprint_enroll(uint8_t id);

uint8_t fingerprint_delete(uint8_t id);

uint8_t fingerprint_search();

uint8_t read_temp_num();

void set_baud_rate_9600();

void read_sys_para();

void fingerprint_task(void *pvParameters);

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

#endif  // FINGERPRINT_DRIVER_H