/**************************************************************************//**
 * @file      servo_driver.c
 * @brief     servo motor driver
 * @author    Tony Yan & Yue Zhang
 * @date      2025-04-16
 ******************************************************************************/

/******************************************************************************
 * Includes
 ******************************************************************************/
#include "servo_driver.h"
#include <asf.h>

/******************************************************************************
 * Variables
 ******************************************************************************/
static struct tcc_module tcc_instance;

/******************************************************************************
 * Global Functions
 ******************************************************************************/
void servo_init(void)
{
	struct tcc_config config_tcc;
	tcc_get_config_defaults(&config_tcc, TCC0);

	// set up prescaler?DIV64?48MHz / 64 = 750kHz?
	config_tcc.counter.clock_prescaler = TCC_CLOCK_PRESCALER_DIV64;
	
	// TOP value = 20ms * (750 ticks/ ms) = 15000
	config_tcc.counter.period = 15000;
	
	// set the compare mode to single-slope PWM
	config_tcc.compare.wave_generation = TCC_WAVE_GENERATION_SINGLE_SLOPE_PWM;
	
	// set compare value: 
	config_tcc.compare.match[2] = 750;

	config_tcc.pins.enable_wave_out_pin[2] = true;
	config_tcc.pins.wave_out_pin[2] = PIN_PA10F_TCC0_WO2;
	config_tcc.pins.wave_out_pin_mux[2] = MUX_PA10F_TCC0_WO2;

	tcc_init(&tcc_instance, TCC0, &config_tcc);
	tcc_enable(&tcc_instance);
}

void pwm_set_servo_angle_unlock_door()
{
	tcc_set_compare_value(&tcc_instance, 2, 750);
}

void pwm_set_servo_angle_lock_door()
{
	tcc_set_compare_value(&tcc_instance, 2, 1400);
}
