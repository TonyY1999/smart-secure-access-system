/**************************************************************************//**
 * @file      fingerprint_driver.h
 * @brief     Header file for fingerprint driver functions in the Smart Secure Access System.
 * @author    Tony Yan & Yue Zhang
 * @date      2025-04-16
 ******************************************************************************/

#ifndef FINGERPRINT_H
#define FINGERPRINT_H 

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdint.h>
#include <stdbool.h>

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
typedef enum
{
    FP_OPERATION_DONE           = 0x00,
    FP_PACKET_RECEIVE_FAIL      = 0x01,
    FP_DETECT_FAIL              = 0x02,
    FP_COLLECT_FAIL             = 0x03,
    FP_IMAGE_MESSY              = 0x06,
    FP_IMAGE_POOR               = 0x07,
    // FP_NOT_MATCH                = 0x08,
    // FP_NOT_FOUND                = 0x09,
    // FP_COMBINE_FAIL             = 0x0A,
    // FP_PAGE_ID_INVALID          = 0x0B,
    // FP_TEMPLATE_INVALID         = 0x0C,
    // FP_UPLOAD_ERROR             = 0x0D,
    // FP_DELETE_FAIL              = 0x10,
    // FP_EMPTY_FAIL               = 0x11,
    // FP_FLASH_ERROR              = 0x18,

    // FP_TIMEOUT                  = 0xF0,
    // FP_BAD_PACKET               = 0xF1,
    // FP_CHECKSUM_ERROR           = 0xF2,
    // FP_PARAM_ERROR              = 0xF3
} fp_status_t;

/******************************************************************************
 * Global Function Declaration
 ******************************************************************************/
/**
 * @brief ...
 *
 * @param[in] ...
 * @param[out] ...
 *
 * @return ...
 */

bool fingerprint_init();

/**
 * @brief Capture a fingerprint image into ImageBuffer.
 * @details Sends the GEN_IMG command to the sensor.
 * @return 0 on success, -1 on failure.
 */
fp_status_t gen_img();

/**
 * @brief Generate character file from captured image into CharBuffer1.
 * @details Sends the IMAGE2TZ1 command to the sensor.
 * @return 0 on success, -1 on failure.
 */
fp_status_t gen_cf_to_b1();

/**
 * @brief Generate character file from captured image into CharBuffer2.
 * @details Sends the IMAGE2TZ2 command to the sensor.
 * @return 0 on success, -1 on failure.
 */
fp_status_t gen_cf_to_b2();

/**
 * @brief Combine CharBuffer1 and CharBuffer2 into a fingerprint template.
 * @details Sends the REG_MODEL command to the sensor.
 * @return 0 on success, -1 on failure.
 */
fp_status_t reg_model();

/**
 * @brief Store the generated fingerprint template into the sensor's flash library.
 * @details Sends the STORE command to the sensor with the specified ID.
 * @param[in] id Fingerprint ID location to store the template.
 * @return 0 on success, -1 on failure.
 */
fp_status_t store_finger(uint8_t id);

/**
 * @brief Enroll a new fingerprint into the library.
 * @details Guides the user through fingerprint capture and storage steps.
 * @param[in] Fingerprint ID that want to be added.
 * @return 0 on success, -1 on failure.
 */
fp_status_t fingerprint_enroll(uint8_t id);

/**
 * @brief Delete a stored fingerprint template.
 * @details Sends the DELETE command to the sensor.
 * @param[in] Fingerprint ID that want to be added. 
 * @return 0 on success, -1 on failure.
 */
fp_status_t fingerprint_delete(uint8_t id);

/**
 * @brief Empty the entire fingerprint library.
 * @details Sends the EMPTY command to the sensor.
 * @return 0 on success, -1 on failure.
 */
fp_status_t fingerprint_empty();

/**
 * @brief Search for a matching fingerprint in the library.
 * @details Captures a fingerprint and compares it to stored templates.
 * @return Matched ID on success, -1 on failure.
 */
fp_status_t fingerprint_search();

/**
 * @brief Read the number of stored fingerprint templates.
 * @details Sends the TEMPLATE_COUNT command to the sensor.
 * @return Number of templates on success, -1 on failure.
 */
fp_status_t read_temp_num();

fp_status_t find_smallest_index();

#ifdef __cplusplus
}
#endif

#endif  // FINGERPRINT_H