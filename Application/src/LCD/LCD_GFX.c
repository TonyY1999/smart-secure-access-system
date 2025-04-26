/*
 * LCD_GFX.c
 *
 * Created: 9/20/2021 6:54:25 PM
 *  Author: You
 */

#include "LCD_GFX.h"
#include "ST7735.h"
 #include <errno.h>

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
 
  #include "Encoder/Encoder.h"
   #include "Buzzer/Buzzer.h"
/******************************************************************************
 * Local Functions
 ******************************************************************************/



/******************************************************************************
 * Global Functions
 ******************************************************************************/

/**************************************************************************//**
* @fn			uint16_t rgb565(uint8_t red, uint8_t green, uint8_t blue)
* @brief		Convert RGB888 value to RGB565 16-bit color data
* @note
*****************************************************************************/

uint16_t rgb565(uint8_t red, uint8_t green, uint8_t blue) {
    return ((((31 * (red + 4)) / 255) << 11) | (((63 * (green + 2)) / 255) << 5) | ((31 * (blue + 4)) / 255));
}

/**************************************************************************//**
* @fn			void LCD_drawPixel(uint8_t x, uint8_t y, uint16_t color)
* @brief		Draw a single pixel of 16-bit rgb565 color to the x & y coordinate
* @note
*****************************************************************************/

void LCD_drawPixel(uint8_t x, uint8_t y, uint16_t color) {
    //LCD_setAddr(x, y, x, y);

	uint8_t color_buf[2];
	color_buf[0] = (color >> 8) & 0xFF;  
	color_buf[1] = color & 0xFF;         

	port_pin_set_output_level(LCD_DC, true); 
	ST7735_WriteBuffer(color_buf, 2);
	
}

//
///**************************************************************************//**
//* @fn			void LCD_setScreen(uint16_t color)
//* @brief		Draw the entire screen to a color
//* @note
//*****************************************************************************/
//

static uint8_t lineBuf[320]; 
void LCD_setScreen(uint16_t color) {
	LCD_setAddr(0, 0, 159, 127);              
	port_pin_set_output_level(LCD_DC, true);  

	uint8_t line[160 * 2];
	for (int i = 0; i < 160; i++) {
		line[2 * i]     = color >> 8;
		line[2 * i + 1] = color & 0xFF;
	}

	for (int j = 0; j < 128; j++) {
		ST7735_WriteBuffer(line, sizeof(line));
	}
}

void LCD_drawBlock(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t color) {
	LCD_setAddr(x0, y0, x1, y1);                    
	port_pin_set_output_level(LCD_DC, true);         

	uint16_t w = x1 - x0 + 1;
	uint16_t h = y1 - y0 + 1;
	uint32_t total_pixels = w * h;

	static uint8_t lineBuf[320 * 2];  

	uint8_t hi = color >> 8;
	uint8_t lo = color & 0xFF;
	for (uint16_t i = 0; i < w; i++) {
		lineBuf[2 * i]     = hi;
		lineBuf[2 * i + 1] = lo;
	}

	for (uint16_t j = 0; j < h; j++) {
		ST7735_WriteBuffer(lineBuf, w * 2);
	}
}

void LCD_drawChar(uint8_t x, uint8_t y, uint16_t character, uint16_t fColor, uint16_t bColor) {
	uint16_t row = character - 0x20;
	if ((LCD_WIDTH - x > 7) && (LCD_HEIGHT - y > 7)) {
		LCD_setAddr(x, y, x + 4, y + 7);  // ???? 5x8 ???????
		port_pin_set_output_level(LCD_DC, true);

		static uint8_t charBuf[5 * 8 * 2];  // 5?? ?? 8?? ?? 2???
		uint8_t *p = charBuf;

		for (int j = 0; j < 8; j++) {
			for (int i = 0; i < 5; i++) {
				uint8_t pixels = ASCII[row][i];
				uint16_t color = (pixels >> j) & 0x01 ? fColor : bColor;
				*p++ = color >> 8;
				*p++ = color & 0xFF;
			}
		}

		ST7735_WriteBuffer(charBuf, 5 * 8 * 2);
	}
}

void LCD_drawString(uint8_t x, uint8_t y, char* str, uint16_t fg, uint16_t bg) {
	while (*str) {
		if (x > LCD_WIDTH - 6) break; 
		LCD_drawChar(x, y, *str, fg, bg);
		x += 6;
		str++;
	}
}

typedef enum {
	MENU_VIEW_FINGERPRINTS = 0,
	MENU_ADD_FINGERPRINT,
	MENU_DELETE_FINGERPRINT,
	MENU_COUNT
} MenuOption;

#define MENU_COUNT 3

const char* menu_items[MENU_COUNT] = {
	"View Fingerprints",
	"Add Fingerprint",
	"Delete Fingerprint"
};

void LCD_drawMenu(uint8_t selected) {

	for (uint8_t i = 0; i < MENU_COUNT; i++) {
		uint8_t y_pos = 10 + i * 40;
		
		uint16_t arrow_fg = (i == selected) ? rgb565(0, 255, 0) : rgb565(255, 255, 255);
		uint16_t text_fg  = arrow_fg;  

		LCD_drawString(4, y_pos, ">", arrow_fg, rgb565(0, 0, 0));  
		LCD_drawString(16, y_pos, menu_items[i], text_fg, rgb565(0, 0, 0)); 
	}
}

#define ROTATION_THRESHOLD 2  

void LCD_handleRotation(int8_t dir, int *selected) {
	static int8_t last_dir = 0;
	static uint8_t same_dir_count = 0;

	if (dir != 0) {
		if (dir == last_dir) {
			same_dir_count++;
			} else {
			same_dir_count = 1;
			last_dir = dir;
		}

		if (same_dir_count >= ROTATION_THRESHOLD) {
			*selected = (*selected + dir + MENU_COUNT) % MENU_COUNT;
			LCD_drawMenu(*selected);
			same_dir_count = 0;
			SerialConsoleWriteString(dir > 0 ? "Encoder: CW\r\n" : "Encoder: CCW\r\n");
		}
	}
}

void handle_view_fingerprint(void) {
	SerialConsoleWriteString(">>> [View Fingerprints Page]\r\n");
	
	LCD_setScreen(rgb565(0, 0, 0));
	LCD_drawString(10, 30, "Fingerprint DB", rgb565(255,255,255), rgb565(0,0,0));
	delay_cycles_ms(1000);


	while (1) {
		if (encoder_button_confirmed()) break;
		vTaskDelay(pdMS_TO_TICKS(20));
	}
	
	LCD_setScreen(rgb565(0, 0, 0));
	LCD_drawMenu(0); 
}
void handle_add_fingerprint(void) {
	SerialConsoleWriteString(">>> Requesting to Add Fingerprint from Cloud\r\n");

	uint8_t finger_id = 1; 

	reset_cloud_permissions(); 
	cloud_request_add(finger_id);

	LCD_setScreen(rgb565(0, 0, 0));
	LCD_drawString(10, 30, "Waiting cloud allow...", rgb565(255,255,255), rgb565(0,0,0));

	uint16_t timeout = 10000; 
	while (timeout--) {
		if (cloud_add_permission_granted()) {
			break;  
		}
		vTaskDelay(pdMS_TO_TICKS(10));
	}

	if (cloud_add_permission_granted()) {
		
		LCD_setScreen(rgb565(0, 0, 0));
		LCD_drawString(10, 30, "Cloud ALLOWED Add", rgb565(0,255,0), rgb565(0,0,0));
		SerialConsoleWriteString("Cloud allowed, start fingerprint enroll!\r\n");
		
		//add fingerprint
		
		port_pin_set_output_level(LED_0_PIN, LED_0_ACTIVE);  
		} else {
		
		LCD_setScreen(rgb565(0, 0, 0));
		LCD_drawString(10, 30, "Cloud DENIED Add", rgb565(255,0,0), rgb565(0,0,0));
		SerialConsoleWriteString("Cloud denied add request.\r\n");
	}

	vTaskDelay(pdMS_TO_TICKS(5000));

	LCD_setScreen(rgb565(0, 0, 0));
	LCD_drawMenu(0); 
}

void handle_delete_fingerprint(void) {
	SerialConsoleWriteString(">>> Requesting to Delete Fingerprint from Cloud\r\n");

	uint8_t finger_id = 1;  

	reset_cloud_permissions();         
	cloud_request_delete(finger_id);   

	LCD_setScreen(rgb565(0, 0, 0));
	LCD_drawString(10, 30, "Waiting cloud allow...", rgb565(255,255,255), rgb565(0,0,0));

	uint16_t timeout = 10000;  
	while (timeout--) {
		if (cloud_delete_permission_granted()) {
			break;
		}
		vTaskDelay(pdMS_TO_TICKS(10));
	}

	if (cloud_delete_permission_granted()) {

		LCD_setScreen(rgb565(0, 0, 0));
		LCD_drawString(10, 30, "Cloud ALLOWED Delete", rgb565(0,255,0), rgb565(0,0,0));
		SerialConsoleWriteString("Cloud allowed delete, now deleting fingerprint...\r\n");
		
		//delete fingerprint
		port_pin_set_output_level(LED_0_PIN, !LED_0_ACTIVE); 
		} else {
		
		LCD_setScreen(rgb565(0, 0, 0));
		LCD_drawString(10, 30, "Cloud DENIED Delete", rgb565(255,0,0), rgb565(0,0,0));
		SerialConsoleWriteString("Cloud denied delete request.\r\n");
	}

	vTaskDelay(pdMS_TO_TICKS(5000));

	LCD_setScreen(rgb565(0, 0, 0));
	LCD_drawMenu(0);
}

void LCD_handleSelection(int selected) {
	SerialConsoleWriteString("Button Pressed!\r\n");

	switch ((MenuOption)selected) {
		case MENU_VIEW_FINGERPRINTS:
		port_pin_set_output_level(LED_0_PIN,LED0_ACTIVE);
		handle_view_fingerprint();
		break;

		case MENU_ADD_FINGERPRINT:
		handle_add_fingerprint();
		break;

		case MENU_DELETE_FINGERPRINT:
		handle_delete_fingerprint();
		break;

		default:
		break;
	}
}

void vLCDTask(void *pvParameters){
	lcd_init();
	delay_cycles_ms(200);

	LCD_setScreen(rgb565(0, 0, 0));
	LCD_drawMenu(0);
	encoder_init();

	int selected = 0;
	
	//port_pin_set_output_level(LED_0_PIN,LED_0_ACTIVE);
	
	//configure_tcc();

	while (1) {
		LCD_handleRotation(encoder_get_rotation(), &selected);

		if (encoder_button_confirmed()) {
			LCD_handleSelection(selected);
		}

		vTaskDelay(pdMS_TO_TICKS(20));
	}
}