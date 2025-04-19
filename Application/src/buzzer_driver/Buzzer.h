/**************************************************************************//**
 * @file      buzzer.h
 * @brief     alarming system buzzer driver
 * @author    Tony Yan & Yue Zhang
 * @date      2025-04-18
 ******************************************************************************/
/**
 * @fn         void configure_tcc(void)
 * @brief      Initializes and configures the TCC peripheral for PWM output.
 * @details    Sets up the Timer/Counter for Control Applications (TCC0) to generate a
 *             single-slope PWM signal with a 4kHz frequency and 50% duty cycle.
 *             Configures output on PA08 using TCC0 WO[0].
 *
 * @param[in]  None
 * @param[out] None
 * @return     None
 * 
 * @note       This function is typically used to drive a buzzer or similar PWM-based device.
 *             Make sure system clocks are configured appropriately before calling this.
 */
void configure_tcc(void);