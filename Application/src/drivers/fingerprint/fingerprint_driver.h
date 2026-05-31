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
#define EMPTY_CMD			{0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x03, 0x0D, 0x00, 0x11};
#define SEARCH_CMD			{0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x08, 0x04, 0x01, 0x00, 0x00, 0x00, 0x32, 0x00, 0x40}
#define READ_SYS_CMD		{0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x03, 0x0F, 0x00, 0x13}
#define TEMP_NUM_CMD		{0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x03, 0x1D, 0x00, 0x21}
#define READ_INDEX_CMD		{0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x04, 0x1F, 0x00, 0x00, 0x24};

/******************************************************************************
 * Structures and Enumerations
 ******************************************************************************/

/******************************************************************************
 * Global Function Declaration
 ******************************************************************************/
/**
 * @brief Initialize the fingerprint sensor hardware and USART interface.
 * @details Configure UART settings and enable callbacks for fingerprint module communication.
 * @return None.
 */
void fingerprint_init();

/**
 * @brief Capture a fingerprint image into ImageBuffer.
 * @details Sends the GEN_IMG command to the sensor.
 * @return 0 on success, -1 on failure.
 */
int8_t gen_img();

/**
 * @brief Generate character file from captured image into CharBuffer1.
 * @details Sends the IMAGE2TZ1 command to the sensor.
 * @return 0 on success, -1 on failure.
 */
int8_t gen_cf_to_b1();

/**
 * @brief Generate character file from captured image into CharBuffer2.
 * @details Sends the IMAGE2TZ2 command to the sensor.
 * @return 0 on success, -1 on failure.
 */
int8_t gen_cf_to_b2();

/**
 * @brief Combine CharBuffer1 and CharBuffer2 into a fingerprint template.
 * @details Sends the REG_MODEL command to the sensor.
 * @return 0 on success, -1 on failure.
 */
int8_t reg_model();

/**
 * @brief Store the generated fingerprint template into the sensor's flash library.
 * @details Sends the STORE command to the sensor with the specified ID.
 * @param[in] id Fingerprint ID location to store the template.
 * @return 0 on success, -1 on failure.
 */
int8_t store_finger(uint8_t id);

/**
 * @brief Enroll a new fingerprint into the library.
 * @details Guides the user through fingerprint capture and storage steps.
 * @param[in] Fingerprint ID that want to be added.
 * @return 0 on success, -1 on failure.
 */
int8_t fingerprint_enroll(uint8_t id);

/**
 * @brief Delete a stored fingerprint template.
 * @details Sends the DELETE command to the sensor.
 * @param[in] Fingerprint ID that want to be added. 
 * @return 0 on success, -1 on failure.
 */
int8_t fingerprint_delete(uint8_t id);

/**
 * @brief Empty the entire fingerprint library.
 * @details Sends the EMPTY command to the sensor.
 * @return 0 on success, -1 on failure.
 */
int8_t fingerprint_empty();

/**
 * @brief Search for a matching fingerprint in the library.
 * @details Captures a fingerprint and compares it to stored templates.
 * @return Matched ID on success, -1 on failure.
 */
int fingerprint_search();

/**
 * @brief Read the number of stored fingerprint templates.
 * @details Sends the TEMPLATE_COUNT command to the sensor.
 * @return Number of templates on success, -1 on failure.
 */
int read_temp_num();

int find_smallest_index();


/**
 * @brief FreeRTOS task to manage fingerprint sensor operations.
 * @details Continuously processes fingerprint enrollment, search, or management operations.
 * @param[in] pvParameters Task input parameters (unused).
 * @return None (task should never return).
 */
void fingerprint_task(void *pvParameters);

#ifdef __cplusplus
}
#endif

#endif  // FINGERPRINT_DRIVER_H