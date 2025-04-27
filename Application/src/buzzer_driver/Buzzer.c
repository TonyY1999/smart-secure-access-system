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

	tcc_reset(&tcc0_instance);

	tcc_get_config_defaults(&config_tcc, TCC0);

	config_tcc.counter.period = 12000;  // 4kHz Buzzer
	config_tcc.compare.wave_generation = TCC_WAVE_GENERATION_SINGLE_SLOPE_PWM;

	config_tcc.compare.match[0] = 0; // Ĭ�Ͼ���
	config_tcc.pins.enable_wave_out_pin[0] = true;
	config_tcc.pins.wave_out_pin[0] = PIN_PA04E_TCC0_WO0;
	config_tcc.pins.wave_out_pin_mux[0] = MUX_PA04E_TCC0_WO0;

	tcc_init(&tcc0_instance, TCC0, &config_tcc);
	tcc_enable(&tcc0_instance);
}

void buzzer_on(void)
{
	SerialConsoleWriteString("Setting buzzer match[0] = 6000\r\n");
	tcc_set_compare_value(&tcc0_instance, 0, 6000); // 50% ????
}

void buzzer_off(void)
{
	tcc_set_compare_value(&tcc0_instance, 0, 0); // ????
}