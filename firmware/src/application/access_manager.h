/**************************************************************************//**
 * @file      fingerprint_manager.h
 * @brief     Fingerprint access manager implementation
 * @author    Tony Yan
 * @date      2026-08-31
 ******************************************************************************/

#ifndef ACCESS_MANAGER_H
#define ACCESS_MANAGER_H 

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Includes
 ******************************************************************************/
#include "fingerprint_service.h"
#include "servo_driver.h"

/******************************************************************************
 * Defines
 ******************************************************************************/
typedef enum {
    ACCESS_CMD_NONE = 0,
    ACCESS_CMD_ENROLL,
    ACCESS_CMD_DELETE,
    ACCESS_CMD_VERIFY,
    ACCESS_CMD_UNLOCK,
    ACCESS_CMD_LOCK
} access_cmd_t;

typedef struct {
    access_cmd_t cmd;
    uint16_t fp_id;
} access_request_t;


/******************************************************************************
 * Structures and Enumerations
 ******************************************************************************/

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
void access_manager_process(access_request_t* request);

#ifdef __cplusplus
}
#endif

#endif  // ACCESS_MANAGER_H