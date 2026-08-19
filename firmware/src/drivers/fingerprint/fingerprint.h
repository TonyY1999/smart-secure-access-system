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
// R503 fingerprint module default settings
#define FP_DEFAULT_ADDRESS      0xFFFFFFFF
#define FP_BAUDRATE             9600

/******************************************************************************
 * Structures and Enumerations
 ******************************************************************************/
// Fingerprint driver status codes
 typedef enum
{
    FP_DRIVER_OK = 0,

    FP_DRIVER_INIT_ERROR,
    FP_DRIVER_UART_ERROR,
    FP_DRIVER_BAD_HEADER,
    FP_DRIVER_BAD_ADDRESS,
    FP_DRIVER_BAD_PID,
    FP_DRIVER_BAD_LENGTH,
    FP_DRIVER_BAD_CHECKSUM,
    FP_DRIVER_BAD_RESPONSE,
    FP_DRIVER_INVALID_ARG

} fp_driver_status_t;

// Fingerprint ACK packet command status codes
typedef enum
{
    FP_STAGE_NONE = 0,

    FP_STAGE_HANDSHAKE,
    FP_STAGE_CHECK_SENSOR,

    FP_STAGE_GET_IMAGE,
    FP_STAGE_GET_IMAGE_EX,
    FP_STAGE_IMAGE_TO_CHAR,
    FP_STAGE_CREATE_MODEL,
    FP_STAGE_STORE_MODEL,

    FP_STAGE_SEARCH,

    FP_STAGE_DELETE,
    FP_STAGE_EMPTY_LIBRARY,
    FP_STAGE_TEMPLATE_COUNT,

    FP_STAGE_LED_CONTROL

} fp_stage_t;

// Fingerprint command execution result structure
typedef struct
{
    fp_driver_status_t driver_status;

    fp_stage_t stage;

    bool confirmation_valid;

    uint8_t confirmation_code;

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
 * @return FP_DRIVER_OK on success, or an error code on failure.
 */
fp_driver_status_t  fp_init(void);


// fp_result_t fp_handshake(void);

// fp_result_t fp_check_sensor(void);

/****************************************
 * Fingerprint basic operations
 ****************************************/
/**
 * @brief Capture a fingerprint image into ImageBuffer.
 * @details Sends the GEN_IMG command to the sensor.
 * @return 0 on success, -1 on failure.
 */
fp_result_t get_img(void);

/**
 * @brief Generate character file from captured image into CharBuffer.
 * @details Sends the IMAGE2TZ1 command to the sensor.
 * @return 0 on success, -1 on failure.
 */
typedef enum
{
    FP_BUFFER_1 = 0x01,
    FP_BUFFER_2 = 0x02

} fp_buffer_id_t;

fp_result_t img_to_char(fp_buffer_id_t buffer_id);

/**
 * @brief Combine CharBuffer1 and CharBuffer2 into a fingerprint template.
 * @details Sends the REG_MODEL command to the sensor.
 * @return 0 on success, -1 on failure.
 */
fp_result_t create_model(void);

/**
 * @brief Store the generated fingerprint template into the sensor's flash library.
 * @details Sends the STORE command to the sensor with the specified ID.
 * @param[in] id Fingerprint ID location to store the template.
 * @return 0 on success, -1 on failure.
 */
fp_result_t store_model(uint8_t id);

/****************************************
 * Search / Verify
 ****************************************/
fp_result_t fp_search(fp_buffer_id_t buffer_id,
                      uint16_t start_page,
                      uint16_t page_num,
                      fp_match_result_t *match);

fp_result_t fp_verify(fp_match_result_t *match);
/**
 * @brief Enroll a new fingerprint into the library.
 * @details Guides the user through fingerprint capture and storage steps.
 * @param[in] Fingerprint ID that want to be added.
 * @return 0 on success, -1 on failure.
 */
fp_result_t fingerprint_enroll(uint8_t id);


/****************************************
 * Fingerprint database
 ****************************************/
/**
 * @brief Delete a stored fingerprint template.
 * @details Sends the DELETE command to the sensor.
 * @param[in] Fingerprint ID that want to be added. 
 * @return 0 on success, -1 on failure.
 */
fp_status_t fp_delete(uint8_t id);

/**
 * @brief Empty the entire fingerprint library.
 * @details Sends the EMPTY command to the sensor.
 * @return 0 on success, -1 on failure.
 */
fp_status_t fp_empty_library(void);

/**
 * @brief Search for a matching fingerprint in the library.
 * @details Captures a fingerprint and compares it to stored templates.
 * @return Matched ID on success, -1 on failure.
 */
fp_status_t fp_search();

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