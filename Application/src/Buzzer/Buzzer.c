/**************************************************************************//**
 * @file      buzzer.c
 * @brief     alarming system buzzer driver
 * @author    Tony Yan & Yue Zhang
 * @date      2025-04-18
 ******************************************************************************/

/******************************************************************************
 * Includes
 ******************************************************************************/
#include "Buzzer.h"
#include <asf.h>

/******************************************************************************
 * Variables
 ******************************************************************************/
struct tcc_module tcc_instance;

/******************************************************************************
 * Local Functions
 ******************************************************************************/
void configure_tcc(void)
{
	struct tcc_config config_tcc;
	tcc_get_config_defaults(&config_tcc, TCC0);

	config_tcc.counter.period = 12000;               // 4kHz
	config_tcc.compare.wave_generation = TCC_WAVE_GENERATION_SINGLE_SLOPE_PWM;
	config_tcc.compare.match[0] = 6000;              // 50% duty cycle

	config_tcc.pins.enable_wave_out_pin[0] = true;
	config_tcc.pins.wave_out_pin[0] = PIN_PA08E_TCC0_WO0;
	config_tcc.pins.wave_out_pin_mux[0] = MUX_PA08E_TCC0_WO0;

	tcc_init(&tcc_instance, TCC0, &config_tcc);
	tcc_enable(&tcc_instance);
}
