/**************************************************************************//**
 * @file      fingerprint_service.c
 * @brief     Fingerprint service implementation
 * @author    Tony Yan
 * @date      2026-08-20
 ******************************************************************************/

/******************************************************************************
 * Includes
 ******************************************************************************/
#include "fingerprint_service.h"
#include "log_print.h"

/******************************************************************************
 * Defines
 ******************************************************************************/
#define FP_CONFIRM_OK   0x00u

#define MAX_RETRY   10u
// 00h: commad execution complete;
// 01h: error when receiving data package;
// 02h: no finger on the sensor;
// 03h: fail to enroll the finger;
// 06h: fail to generate character file due to the over-disorderly fingerprint image;
// 07h: fail to generate character file due to lackness of character point or over-smallness of
// fingerprint image
// 08h: finger doesn’t match;
// 09h: fail to find the matching finger;
// 0Ah: fail to combine the character files;
// 0Bh: addressing PageID is beyond the finger library;
// 0Ch: error when reading template from library or the template is invalid;
// 0Dh: error when uploading template;
// 0Eh: Module can’t receive the following data packages.
// 0Fh: error when uploading image;
// 10h: fail to delete the template;
// 11h: fail to clear finger library;
// 13h: wrong password!
// 15h: fail to generate the image for the lackness of valid primary image;
// 18h: error when writing flash;
// 19h: No definition error;
// 1Ah: invalid register number;
// 1Bh: incorrect configuration of register;
// 1Ch: wrong notepad page number;
// 1Dh: fail to operate the communi

/******************************************************************************
 * Variables
 ******************************************************************************/

/******************************************************************************
 * Forward Declarations
 ******************************************************************************/
static fp_result_t capture_template(fp_buffer_id_t buffer);

/******************************************************************************
 * Global Functions
 ******************************************************************************/
fp_result_t fp_enroll(void) {
    fp_result_t result = {FP_STATUS_OK, FP_INVALID_CONFIRMATION_CODE};

    // Capture fingerprint template from buffer 1 and buffer 2
    result = capture_template(FP_CHAR_BUFFER_1, MAX_RETRY);
    if(result.status != FP_STATUS_OK || result.confirmation_code != FP_CONFIRM_OK) {
        return result;
    }

    result = capture_template(FP_CHAR_BUFFER_2, MAX_RETRY);
    if(result.status != FP_STATUS_OK || result.confirmation_code != FP_CONFIRM_OK) {
        return result;
    }

    // Create fingerprint model from the two templates
    log_info("Createing fingerprint model...");
    result = create_model();
    if(result.status != FP_STATUS_OK || result.confirmation_code != FP_CONFIRM_OK) {
        log_error("Failed to create fingerprint model.");
        return result;
    }
    log_info("Success created fingerprint model.");

    // Store the fingerprint model into the library
    log_info("Storing fingerprint model to library");
    uint16_t temp_nums = 0;
    result = fp_read_temp_num(&temp_nums);
    if(result.status != FP_STATUS_OK || result.confirmation_code != FP_CONFIRM_OK) {
        log_error("Failed to read template number.");
        return result;
    }
    
    result = store_model(temp_nums);
    if(result.status != FP_STATUS_OK || result.confirmation_code != FP_CONFIRM_OK) {
        log_error("Failed to store fingerprint model.");
        return result;
    }
    log_info("Success stored fingerprint model into library.");

    return result;
}

/******************************************************************************
 * Local Functions
 ******************************************************************************/
static fp_result_t capture_template(fp_buffer_id_t buffer, uint8_t max_retry)
{
    fp_result_t result = {FP_STATUS_OK, FP_INVALID_CONFIRMATION_CODE};

    log_info("Please place your finger on the sensor.");

    for (uint8_t retry = 0; retry < max_retry; retry++)
    {
        result = get_img();

        if (result.status != FP_STATUS_OK)
        {
            return result;
        }

        if (result.confirmation_code == FP_CONFIRM_OK)
        {
            log_info("Fingerprint image captured successfully.");
            break;
        }

        log_error("No valid fingerprint detected. Please try again.");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    // maximum retry reached
    if(result.status != FP_STATUS_OK) {
        log_error("Fingerprint capture timeout.");
        return result;
    }

    log_info("Generating fingerprint template...");

    result = img_to_char(buffer);

    if (result.status != FP_STATUS_OK)
    {
        return result;
    }

    if (result.confirmation_code != FP_CONFIRM_OK)
    {
        log_error("Failed to generate fingerprint template.");
        return result;
    }

    log_info("Template generated successfully.");

    return result;
}

/******************************************************************************
 * Callback Functions
 ******************************************************************************/
