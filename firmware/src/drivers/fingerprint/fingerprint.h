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

/******************************************************************************
 * Structures and Enumerations
 ******************************************************************************/
typedef enum
{
    FP_OK = 0,
    FP_ERR_UART_READ,
    FP_ERR_UART_WRITE,
    FP_ERR_WRONG_PID,
    FP_ERR_ON_PASS_IN_ARGUMENT,
} fp_status_t;

typedef struct
{
    fp_status_t status;
    uint8_t confirmation_code;
    uint16_t fp_id;
    // uint16_t score;
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
bool fp_init(void);


// fp_result_t fp_handshake(void);

// fp_result_t fp_check_sensor(void);

/****************************************
 * Fingerprint basic operations
 ****************************************/
/**
 * @brief Capture a fingerprint image an put it into ImageBuffer.
 * @details Sends the command with the FP_CMD_GET_IMAGE instruction code to the sensor.
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
uint8_t fp_search(fp_buffer_id_t buffer_id,
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