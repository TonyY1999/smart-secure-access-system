#include "CliThread/CliThread.h"
#include "FreeRTOS.h"
#include "I2cDriver\I2cDriver.h"
#include "SerialConsole.h"
#include "WifiHandlerThread/WifiHandler.h"
#include "asf.h"
#include "driver/include/m2m_wifi.h"
#include "main.h"
#include "stdio_serial.h"
#include "SerialConsole/SerialConsole.h"
#include "imu_driver/adxl345_imu.h"
#include "MCHP_ATWx.h"

#include "Buzzer.h"
 
/******************************************************************************
 * Local Functions
 ******************************************************************************/

struct tcc_module tcc0_instance;

void buzzer_init(void)
{
	struct tcc_config config_tcc;
	struct system_pinmux_config pin_config;
	
	system_apb_clock_set_mask(SYSTEM_CLOCK_APB_APBC, PM_APBCMASK_TCC0);
	system_gclk_chan_enable(TCC0_GCLK_ID);
	
	tcc_reset(&tcc0_instance);

	tcc_get_config_defaults(&config_tcc, TCC0);
	config_tcc.counter.clock_prescaler = TCC_CLOCK_PRESCALER_DIV1;
	config_tcc.counter.period = 12000;  // 4kHz Buzzer
	config_tcc.compare.wave_generation = TCC_WAVE_GENERATION_SINGLE_SLOPE_PWM;

	config_tcc.compare.match[0] = 0; 
	config_tcc.pins.enable_wave_out_pin[0] = true;
	config_tcc.pins.wave_out_pin[0] = PIN_PA04E_TCC0_WO0;
	config_tcc.pins.wave_out_pin_mux[0] = MUX_PA04E_TCC0_WO0;
	
	tcc_init(&tcc0_instance, TCC0, &config_tcc);
	
	    system_pinmux_get_config_defaults(&pin_config);
	    pin_config.mux_position = MUX_PA04E_TCC0_WO0;
	    pin_config.direction    = SYSTEM_PINMUX_PIN_DIR_OUTPUT;
	    system_pinmux_pin_set_config(PIN_PA04E_TCC0_WO0, &pin_config);
	
	tcc_enable(&tcc0_instance);
}

void buzzer_on(void)
{
	if (!(tcc0_instance.hw->CTRLA.reg & TCC_CTRLA_ENABLE)) {
		SerialConsoleWriteString("[Debug] Warning: TCC0 not enabled before buzzer_on!\r\n");
		buzzer_init();
	}
	
	SerialConsoleWriteString("Setting buzzer match[0] = 6000\r\n");
	tcc_set_compare_value(&tcc0_instance, 0, 8000); // 50% ????
}

void buzzer_off(void)
{
	tcc_set_compare_value(&tcc0_instance, 0, 0); // ????
}