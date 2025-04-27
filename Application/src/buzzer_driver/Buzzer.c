


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

void buzzer_on(void)
{
	SerialConsoleWriteString("Setting buzzer match[0] = 6000\r\n");
	//tcc_set_compare_value(&tcc_instance_tcc0, 0, 8500); // 50% ????
}

void buzzer_off(void)
{
	//tcc_set_compare_value(&tcc_instance_tcc0, 0, 0); // ????
}