
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


#ifndef ENCODER_H_
#define ENCODER_H_

#define ENCODER_A_PIN      PIN_PA20
#define ENCODER_B_PIN      PIN_PA21
#define ENCODER_BUTTON_PIN PIN_PB11  //PCBA PB22

void encoder_init(void);
int8_t encoder_get_rotation(void);  // 返回 +1, -1, or 0
bool encoder_button_pressed(void);  // 可选：读取按钮按下状态
bool encoder_button_confirmed(void);

#endif /* ENCODER_H_ */