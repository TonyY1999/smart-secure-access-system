/**************************************************************************//**
 * @file      fingerprint_service.h
 * @brief     Fingerprint service implementation
 * @author    Tony Yan
 * @date      2026-08-20
 ******************************************************************************/

#ifndef FINGERPRINT_SERVICE_H
#define FINGERPRINT_SERVICE_H   

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Includes
 ******************************************************************************/
#include "fingerprint.h"

/******************************************************************************
 * Defines
 ******************************************************************************/

/******************************************************************************
 * Structures and Enumerations
 ******************************************************************************/

/******************************************************************************
 * Global Function Declaration
 ******************************************************************************/
/**
 * @brief Enroll a new fingerprint into the library.
 *
 * @param[in] None
 * @param[out] None
 *
 * @return fp_result_t structure containing the status and confirmation code.
 */
fp_result_t fp_enroll(void);

 /**
 * @brief ...
 *
 * @param[in] ...
 * @param[out] ...
 *
 * @return ...
 */
// fingerprint_authenticate();



//fingerprint_delete();

#ifdef __cplusplus
}
#endif

#endif  // FINGERPRINT_SERVICE_H 