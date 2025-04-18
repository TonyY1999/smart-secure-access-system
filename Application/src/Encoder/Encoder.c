/*
 * LCD_GFX.c
 *
 * Created: 9/20/2021 6:54:25 PM
 *  Author: You
 */


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
 
#include "encoder.h"
/******************************************************************************
 * Local Functions
 ******************************************************************************/


static volatile int8_t encoder_delta = 0;
static volatile bool button_pressed_flag = false;
static volatile bool button_pending = false;

void encoder_a_callback(void) {
	bool b_state = port_pin_get_input_level(ENCODER_B_PIN);
	if (b_state) {
	 encoder_delta--;
	 } else {
	 encoder_delta++;
	 }
}

void encoder_button_callback(void) {
	button_pending = true;  
}

void encoder_init(void) {
	struct port_config pin_conf;
	port_get_config_defaults(&pin_conf);
	pin_conf.direction  = PORT_PIN_DIR_INPUT;
	pin_conf.input_pull = PORT_PIN_PULL_UP;

	port_pin_set_config(ENCODER_A_PIN, &pin_conf);
	port_pin_set_config(ENCODER_B_PIN, &pin_conf);
	port_pin_set_config(ENCODER_BUTTON_PIN, &pin_conf);

	// signal A callback: rising edge
	struct extint_chan_conf config_extint_chan;
	extint_chan_get_config_defaults(&config_extint_chan);
	config_extint_chan.gpio_pin           = ENCODER_A_PIN;
	config_extint_chan.gpio_pin_mux       = MUX_PA20A_EIC_EXTINT4;
	config_extint_chan.detection_criteria = EXTINT_DETECT_RISING;
	config_extint_chan.filter_input_signal = true;
	extint_chan_set_config(4, &config_extint_chan);
	extint_register_callback(encoder_a_callback, 4, EXTINT_CALLBACK_TYPE_DETECT);
	extint_chan_enable_callback(4, EXTINT_CALLBACK_TYPE_DETECT);

	// PB11 callback register
	extint_chan_get_config_defaults(&config_extint_chan);
	config_extint_chan.gpio_pin           = ENCODER_BUTTON_PIN;
	config_extint_chan.gpio_pin_mux       = MUX_PB11A_EIC_EXTINT11;
	config_extint_chan.detection_criteria = EXTINT_DETECT_FALLING;
	config_extint_chan.filter_input_signal = true;
	extint_chan_set_config(11, &config_extint_chan);
	extint_register_callback(encoder_button_callback, 11, EXTINT_CALLBACK_TYPE_DETECT);
	extint_chan_enable_callback(11, EXTINT_CALLBACK_TYPE_DETECT);
}

int8_t encoder_get_rotation(void) {
	//SerialConsoleWriteString("Get rotation called\r\n");
	int8_t val = encoder_delta;
	encoder_delta = 0;  
	return val;
}

bool encoder_button_pressed(void) {
	if (button_pressed_flag) {
		button_pressed_flag = false; 
		return true;
	}
	return false;
}

bool encoder_button_confirmed(void) {
	static bool waiting_release = false;

	if (button_pending) {
		button_pending = false;

		if (!port_pin_get_input_level(ENCODER_BUTTON_PIN)) {
			waiting_release = true;
			return false;  
		}
	}

	if (waiting_release && port_pin_get_input_level(ENCODER_BUTTON_PIN)) {
		waiting_release = false;
		return true;  
	}

	return false;
}