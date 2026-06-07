/**************************************************************************//**
 * @file      servo_driver.h
 * @brief     Header file for servo driver functions in the Smart Secure Access System.
 * @author    Tony Yan & Yue Zhang
 * @date      2025-04-16
 ******************************************************************************/

#ifndef SERVO_DRIVER_H
#define SERVO_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdbool.h>

/******************************************************************************
 * Global Function Declaration
 ******************************************************************************/
/**
 * @brief Initializes the servo motor by configuring the TCC module for PWM generation.
 *
 * @param[in] None
 * @param[out] None
 *
 * @returns true if initialization was successful, false otherwise.
 */
bool servo_init(void);

/**
     * @brief Locks the servo motor by setting the PWM pulse width.
     *
 * @param[in] None
 * @param[out] None
 *
 * @returns None
 */
void servo_lock(void);

/**
 * @brief Unlocks the servo motor by setting the PWM pulse width.
 *
 * @param[in] None
 * @param[out] None
 *
 * @returns None
 */
void servo_unlock(void);

#ifdef __cplusplus
}
#endif

#endif  // SERVO_DRIVER_H