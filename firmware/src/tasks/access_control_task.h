/**************************************************************************//**
 * @file      access_control_task.c
 * @brief     Source file for access_control_task.c, implementing the access control task in the smart secure access system.
 * @author    Eduardo Garcia
 * @date      2020-01-01
 ******************************************************************************/

#ifndef ACCESS_CONTROL_TASK_H
#define ACCESS_CONTROL_TASK_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Includes
 ******************************************************************************/

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
 * @brief FreeRTOS task to manage fingerprint sensor operations.
 * @details Continuously processes fingerprint enrollment, search, or management operations.
 * @param[in] pvParameters Task input parameters (unused).
 * @return None (task should never return).
 */
void fingerprint_task(void *pvParameters);


#ifdef __cplusplus
}
#endif

#endif  //ACCESS_CONTROL_TASK_H