/**************************************************************************//**
 * @file      fingerprint_driver.h
 * @brief     Header file for fingerprint driver functions in the Smart Secure Access System.
 * @author    Tony Yan & Yue Zhang
 * @date      2026-08-02
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
#define FP_INVALID_CONFIRMATION_CODE    0x1Eu
#define FP_INVALID_FINGERPRINT_ID       0xFFFFu

/** 
 * A fingerprint data packet consists of a fixed header, address, PID, length, content, and checksum.
 * 
 * Header (2 bytes)
 * Address (4 bytes)
 * Packet identifier (1 byte) 
 * Packet Length (2 bytes) 
 * Packet Content (N bytes)
 * Checksum (2 bytes)

 * Header: Fixed value of 0xEF01; High byte transferred first.
 * Address: Fixed value of 0xFFFFFFFF; High byte transferred first.
 * Packet identifier: 0x01 for command packets, 0x07 for acknowledgment packets.
 * Packet Length: length of package content plus the length of Checksum( 2 bytes), unit is byte. High byte transferred first.
 * Packet Content: Command or data specific to the packet type; variable length.
 * Checksum: Arithmetic sum of package identifier, package length and all package contens, high byte is transferred first
**/

/******************************************************************************
 * Structures and Enumerations
 ******************************************************************************/
// Fingerprint driver status enumeration
typedef enum
{
    FP_STATUS_OK = 0,

    // Parameter errors
    FP_ERR_INVALID_CONTENT,
    FP_ERR_INVALID_CONTENT_LENGTH,
    FP_ERR_INVALID_INSTRUCTION_CODE,

    // UART errors
    FP_ERR_UART_SEND,
    FP_ERR_UART_RECEIVE,

    // Packet errors
    FP_ERR_HEADER,
    FP_ERR_ADDRESS,
    FP_ERR_CHECKSUM,

    // Protocol errors
    // FP_ERR_UNEXPECTED_INSTRUCTION,
    // FP_ERR_CONFIRMATION_CODE,
} fp_status_t;

// Fingerprint match result structure
typedef struct
{
    fp_status_t status;
    uint8_t confirmation_code;
    uint16_t fp_id;
} fp_result_t;

/******************************************************************************
 * Global Function Declaration
 ******************************************************************************/
/**
 * @brief Initialize the fingerprint module hardware and USART interface.
 *
 * @param[in] None
 * @param[out] None
 *
 * @return
 */
bool fp_init(void);

/****************************************
 * Fingerprint basic operations
 ****************************************/
/**
 * @brief Capture a fingerprint image from the sensor.
 *
 * @param[in] None
 * @param[out] None
 *
 * @return fp_result_t structure containing the status and confirmation code.
 */
fp_result_t get_img(void);


typedef enum
{
    FP_CHAR_BUFFER_1 = 0x01u,
    FP_CHAR_BUFFER_2 = 0x02u

} fp_buffer_id_t;

/**
 * @brief Convert the captured fingerprint image to a character file and store it in the specified buffer.
 *
 * @param[in] buffer_id The ID of the buffer where the character file will be stored.
 * @param[out] None
 *
 * @return fp_result_t structure containing the status and confirmation code.
 */
fp_result_t img_to_char(fp_buffer_id_t buffer_id);

/**
 * @brief Create a fingerprint model by combining two character files from the buffers.
 *
 * @param[in] None
 * @param[out] None
 *
 * @return fp_result_t structure containing the status and confirmation code.
 */
fp_result_t create_model(void);

/**
 * @brief Store the created fingerprint model in the sensor's library with the specified ID.
 *
 * @param[in] fp_id The fingerprint ID under which to store the fingerprint model.
 * @param[out] None
 *
 * @return fp_result_t structure containing the status and confirmation code.
 */
fp_result_t store_model(uint16_t fp_id);

/****************************************
 * Search / Verify
 ****************************************/
uint8_t fp_search(fp_buffer_id_t buffer_id,
                      uint16_t start_page,
                      uint16_t page_num,
                      fp_match_result_t *match);

fp_result_t fp_verify(fp_match_result_t *match);
/**
 * @brief Enroll a new fingerprint into the library.
 * @details Guides the user through fingerprint capture and storage steps.
 * @param[in] Fingerprint ID that want to be added.
 * @return fp_result_t structure containing the status and confirmation code    .
 */
fp_result_t fingerprint_enroll(uint8_t id);


/****************************************
 * Fingerprint database
 ****************************************/
/**
 * @brief Delete a stored fingerprint template.
 * @details Sends the DELETE command to the sensor.
 * @param[in] Fingerprint ID that want to be added. 
 * @return fp_result_t structure containing the status and confirmation code.
 */
fp_result_t fp_delete(uint8_t id);

/**
 * @brief Empty the entire fingerprint library.
 * @details Sends the EMPTY command to the sensor.
 * @return fp_result_t structure containing the status and confirmation code.
 */
fp_result_t fp_empty_library(void);

/**
 * @brief Search for a matching fingerprint in the library.
 * @details Captures a fingerprint and compares it to stored templates.
 * @return fp_result_t structure containing the status and confirmation code.
 */
fp_result_t fp_search();

/**
 * @brief Read the number of stored fingerprint templates.
 * @details Sends the TEMPLATE_COUNT command to the sensor.
 * @return Number of templates on success, -1 on failure.
 */
fp_status_t fp_read_temp_num();

fp_status_t fp_find_smallest_index();

#ifdef __cplusplus
}
#endif

#endif  // FINGERPRINT_H
