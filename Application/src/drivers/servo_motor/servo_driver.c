/**************************************************************************//**
 * @file      servo_driver.c
 * @brief     Source file for servo driver functions in the Smart Secure Access System.
 * @author    Tony Yan & Yue Zhang
 * @date      2025-04-16
 ******************************************************************************/

/******************************************************************************
 * Includes
 ******************************************************************************/
#include "servo_driver.h"
#include "tcc.h"

/******************************************************************************
 * Defines
 ******************************************************************************/
#define SERVO_TCC_MODULE			TCC0
#define SERVO_WO_CHANNEL            1

#define SERVO_WAVE_OUT_PIN          PIN_PA05E_TCC0_WO1
#define SERVO_WAVE_OUT_MUX          MUX_PA05E_TCC0_WO1

#define GCLK_FREQ_HZ            	48000000UL
#define PRESCALER_DIV           	64UL
#define TIMER_FREQ_HZ           	(GCLK_FREQ_HZ / PRESCALER_DIV)

#define SERVO_PWM_PERIOD_US         20000UL

#define SERVO_UNLOCK_PULSE_US       600U  // 1ms pulse width corresponds to -90 degree (unlock position) on datasheet, but we found 600us works better for our servo to fully unlock
#define SERVO_LOCK_PULSE_US         1500U  // 1.5ms pulse width corresponds to 0 degree (lock position)

/******************************************************************************
 * Variables
 ******************************************************************************/
static struct tcc_module tcc0_instance;  // TCC instance for TCC0

/******************************************************************************
 * Forward Declarations
 ******************************************************************************/
/**
 * @brief Converts microseconds to timer ticks.
 *
 * @param[in] time_us The time in microseconds.
 * @param[out] None
 *
 * @returns The equivalent time in timer ticks.
 */
static uint32_t servo_us_to_ticks(uint32_t time_us);

/**
 * @brief Sets the PWM pulse width for the servo motor.
 *
 * @param[in] pulse_us The desired pulse width in microseconds.
 * @param[out] None
 *
 * @returns None
 */
static void servo_set_pulse_us(uint16_t pulse_us);

/******************************************************************************
 * Global Functions
 ******************************************************************************/
bool servo_init(void)
{
	struct tcc_config tcc_config_instance;

	// Reset TCC module to ensure it's in a known state before configuration
	// tcc_reset(&tcc0_instance);

	tcc_get_config_defaults(&tcc_config_instance, SERVO_TCC_MODULE);

	// set up prescaler DIV64, 48MHz / 64 = 750kHz
	tcc_config_instance.counter.clock_prescaler = TCC_CLOCK_PRESCALER_DIV64;
	
	// TOP value = 20ms * (750 ticks/ ms) = 15000 ticks
	tcc_config_instance.counter.period = servo_us_to_ticks(SERVO_PWM_PERIOD_US);
	
	// set the compare mode to single-slope PWM
	tcc_config_instance.compare.wave_generation = TCC_WAVE_GENERATION_SINGLE_SLOPE_PWM;
	
	// configure the output pin for the servo PWM signal
	tcc_config_instance.pins.enable_wave_out_pin[SERVO_WO_CHANNEL] = true;
	tcc_config_instance.pins.wave_out_pin[SERVO_WO_CHANNEL] = SERVO_WAVE_OUT_PIN;
	tcc_config_instance.pins.wave_out_pin_mux[SERVO_WO_CHANNEL] = SERVO_WAVE_OUT_MUX;

	// set compare value initially(lock the door)
	tcc_config_instance.compare.match[SERVO_WO_CHANNEL] = servo_us_to_ticks(SERVO_LOCK_PULSE_US);

	// initialize the TCC module with the specified configuration
	if(tcc_init(&tcc0_instance, SERVO_TCC_MODULE, &tcc_config_instance) != STATUS_OK) {
		return false;
	}
	
	// Enable the TCC module to start generating the PWM signal
	tcc_enable(&tcc0_instance);

	return true;
}

void servo_lock(void)
{
	servo_set_pulse_us(SERVO_LOCK_PULSE_US);
}

void servo_unlock(void)
{
	servo_set_pulse_us(SERVO_UNLOCK_PULSE_US);
}

/******************************************************************************
 * Local Functions
 ******************************************************************************/
static uint32_t servo_us_to_ticks(uint32_t time_us)
{
    return (time_us * TIMER_FREQ_HZ) / 1000000UL;
}

static void servo_set_pulse_us(uint16_t pulse_us) 
{
	tcc_set_compare_value(&tcc0_instance, SERVO_WO_CHANNEL, servo_us_to_ticks(pulse_us));
}

/******************************************************************************
 * Callback Functions
 ******************************************************************************/