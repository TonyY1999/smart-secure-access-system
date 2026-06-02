/**************************************************************************//**
 * @file      access_control.h
 * @brief     Header file for access_control.c, defining the interface for access control operations in the smart secure access system.
 * @author    Eduardo Garcia
 * @date      2020-01-01
 ******************************************************************************/

#ifndef ACCESS_CONTROL_H
#define ACCESS_CONTROL_H 

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

/******************************************************************************
 * Structures and Enumerations
 ******************************************************************************/
typedef enum
{
    ACCESS_SOURCE_FINGERPRINT = 0,
    ACCESS_SOURCE_REMOTE,
    ACCESS_SOURCE_IMU,
    ACCESS_SOURCE_ENCODER
} access_source_t;

typedef enum
{
    ACCESS_EVENT_UNLOCK_REQUEST = 0,
    ACCESS_EVENT_ACCESS_DENIED,
    ACCESS_EVENT_TAMPER_ALERT,
    ACCESS_EVENT_SYSTEM_ERROR
} access_event_type_t;

typedef struct
{
    access_event_type_t type;
    access_source_t source;
    int user_id;
    int error_code;
} access_event_t;


/******************************************************************************
 * Global Function Declaration
 ******************************************************************************/
/**
 * @fn		int ExampleFuncionComment(int inputInt, void *pvParameters)
 * @brief	An example function comment. Erase me!
 * @details 	Write details of function here.

 * @param[in]	InputInt Use me to describe inputs to functions
 * @param[out]	*pvParameters Use me to describe outputs of functions passed as arguments
 * @return		Use me to explain the return of an argument.
 * @note
 */
void access_control_init(void);


void access_control_handle_event(const access_event_t *event);


#ifdef __cplusplus
}
#endif

#endif  //ACCESS_CONTROL_H