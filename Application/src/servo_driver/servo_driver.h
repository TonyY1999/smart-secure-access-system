/**************************************************************************//**
 * @file      servo_driver.h
 * @brief     servo motor driver
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
#include <stdint.h>

/******************************************************************************
 * Global Function Declaration
 ******************************************************************************/
/**
 * @fn         void servo_init()
 * @brief      Initializes the PWM peripheral (TCC0 on PA10) for servo control.
 * @details    Configures TCC0 waveform output 2 (WO[2]) on PA10 for 50Hz PWM,
 *             suitable for standard servo motor control. Assumes system clock is already initialized.
 *
 * @return     None
 * @note       This function should be called before using any pwm_set_servo_angle().
 */
void servo_init();

/**
 * @fn         void pwm_set_servo_angle_unlock_door()
 * @brief      Moves the servo to the unlocked position (typically 0°).
 * @details    Sets the PWM compare value to generate a 1ms pulse width,
 *             corresponding to the servo's 0° position. This unlocks the door
 *             in a smart lock system.
 *
 * @param      None
 * @return     None
 * @note       Ensure pwm_init() has been called prior to using this function.
 */
void pwm_set_servo_angle_unlock_door();

/**
 * @fn         void pwm_set_servo_angle_lock_door()
 * @brief      Moves the servo to the locked position (typically 90°).
 * @details    Sets the PWM compare value to generate a 2ms pulse width,
 *             corresponding to the servo's 90s° position. This locks the door
 *             in a smart lock system.
 *
 * @param      None
 * @return     None
 * @note       Ensure pwm_init() has been called prior to using this function.
 */
void pwm_set_servo_angle_lock_door();

#ifdef __cplusplus
}
#endif

#endif  // SERVO_DRIVER_H