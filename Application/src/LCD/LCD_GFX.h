
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
 #include "ASCII_LUT.h"
 #include "Encoder/Encoder.h"

#ifndef LCD_GFX_H_
#define LCD_GFX_H_

#define LCD_WIDTH  160
#define LCD_HEIGHT 128

// colors
#define	BLACK     0x0000
#define WHITE     0xFFFF
#define	BLUE      0x001F
#define	RED       0xF800
#define	GREEN     0x07E0
#define CYAN      0x07FF
#define MAGENTA   0xF81F
#define YELLOW    0xFFE0



void vLCDTask(void *pvParameters);

uint16_t rgb565(uint8_t red, uint8_t green, uint8_t blue);
void LCD_drawPixel(uint8_t x, uint8_t y, uint16_t color);
void LCD_drawChar(uint8_t x, uint8_t y, uint16_t character, uint16_t fColor, uint16_t bColor);
void LCD_drawCircle(uint8_t x0, uint8_t y0, uint8_t radius,uint16_t color);
void LCD_drawLine(short x0,short y0,short x1,short y1,uint16_t c);
void LCD_drawBlock(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1,uint16_t color);
void LCD_setScreen(uint16_t color);
void LCD_drawString(uint8_t x, uint8_t y, char* str, uint16_t fg, uint16_t bg);
void LCD_drawMenu(uint8_t selected);

#endif /* LCD_GFX_H_ */