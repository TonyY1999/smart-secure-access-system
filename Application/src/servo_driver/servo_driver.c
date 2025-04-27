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
extern struct tcc_module tcc0_instance;

/******************************************************************************
 * Global Functions
 ******************************************************************************/
//void config_servo()
//{
	//struct tcc_config config_tcc;
	//tcc_get_config_defaults(&config_tcc, TCC0);
//
	//// set up prescaler?DIV64?48MHz / 64 = 750kHz?
	//config_tcc.counter.clock_prescaler = TCC_CLOCK_PRESCALER_DIV64;
	//
	//// TOP value = 20ms * (750 ticks/ ms) = 15000
	//config_tcc.counter.period = 15000;
	//
	//// set the compare mode to single-slope PWM
	//config_tcc.compare.wave_generation = TCC_WAVE_GENERATION_SINGLE_SLOPE_PWM;
	//
	//// set compare value: 
	//config_tcc.compare.match[1] = 750;
//
	//config_tcc.pins.enable_wave_out_pin[1] = true;
	//config_tcc.pins.wave_out_pin[1] = PIN_PA05E_TCC0_WO1;
	//config_tcc.pins.wave_out_pin_mux[1] = MUX_PA05E_TCC0_WO1;
//
	//tcc_init(&tcc_instance, TCC0, &config_tcc);
	//tcc_enable(&tcc_instance);
//}

void config_servo()
{
	struct tcc_config config_tcc;
	tcc_reset(&tcc0_instance);
	tcc_get_config_defaults(&config_tcc, TCC0);

	// set up prescaler?DIV64?48MHz / 64 = 750kHz?
	config_tcc.counter.clock_prescaler = TCC_CLOCK_PRESCALER_DIV64;
	
	// TOP value = 20ms * (750 ticks/ ms) = 15000
	config_tcc.counter.period = 15000;
	
	// set the compare mode to single-slope PWM
	config_tcc.compare.wave_generation = TCC_WAVE_GENERATION_SINGLE_SLOPE_PWM;
	
	// set compare value:
	config_tcc.compare.match[1] = 750;

	config_tcc.pins.enable_wave_out_pin[1] = true;
	config_tcc.pins.wave_out_pin[1] = PIN_PA05E_TCC0_WO1;
	config_tcc.pins.wave_out_pin_mux[1] = MUX_PA05E_TCC0_WO1;

	tcc_init(&tcc0_instance, TCC0, &config_tcc);
	tcc_enable(&tcc0_instance);
}

void pwm_set_servo_angle_unlock_door()
{
	tcc_set_compare_value(&tcc0_instance, 1, 750);
}

void pwm_set_servo_angle_lock_door()
{
	tcc_set_compare_value(&tcc0_instance, 1, 1400);
}

void servo_task(void *pvParameters){
	config_servo();
	
	while (1)
	{
		vTaskSuspend(NULL);
		
		pwm_set_servo_angle_unlock_door();
		vTaskDelay(pdMS_TO_TICKS(5000));

		pwm_set_servo_angle_lock_door();
	}
}
