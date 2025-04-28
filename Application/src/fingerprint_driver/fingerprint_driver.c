/**************************************************************************//**
 * @file      fingerprint_driver.c
 * @brief     fingerprint module driver
 * @author    Tony Yan & Yue Zhang
 * @date      2025-04-16
 ******************************************************************************/

/******************************************************************************
 * Includes
 ******************************************************************************/
#include "fingerprint_driver.h"

#include <errno.h>

#include "FreeRTOS.h"
#include "I2cDriver\I2cDriver.h"
#include "asf.h"
#include "driver/include/m2m_wifi.h"
#include "main.h"
#include "stdio_serial.h"
#include "SerialConsole/SerialConsole.h"
#include "MCHP_ATWx.h"

#include "CliThread/CliThread.h"
#include "WifiHandlerThread/WifiHandler.h"
#include "servo_driver/servo_driver.h"
#include "fingerprint_driver/fingerprint_driver.h"
#include "imu_driver/adxl345_imu.h"
#include "LCD/ST7735.h"
#include "LCD/LCD_GFX.h"

#include "buzzer_driver/Buzzer.h"

#include <asf.h>

/******************************************************************************
 * Defines
 ******************************************************************************/
#define CHECK_SUCCESS(x) do { if ((x) != 0) return -1; } while(0)

/******************************************************************************
 * Variables
 ******************************************************************************/
static struct usart_module fingerprint_usart_instance;
static SemaphoreHandle_t rx_semaphore;

extern TaskHandle_t servoTaskHandle;

uint8_t add_id = 0;
uint8_t delete_id = 0;

/******************************************************************************
 * Forward Declarations
 ******************************************************************************/
static void fingerprint_send_packet(uint8_t *packet, uint16_t length);
static int fingerprint_read_response(uint8_t *buffer, uint16_t length);

static check_success(uint8_t res);

static void fingerprint_read_callback(struct usart_module *const usart_module);

/******************************************************************************
 * Global Functions
 ******************************************************************************/
void fingerprint_init()
{	
	struct usart_config config_usart;
	usart_get_config_defaults(&config_usart);

	config_usart.baudrate = 9600;
	config_usart.mux_setting = USART_RX_1_TX_0_XCK_1;
	config_usart.pinmux_pad0 = PINMUX_PA08C_SERCOM0_PAD0;  // TX
	config_usart.pinmux_pad1 = PINMUX_PA09C_SERCOM0_PAD1;  // RX
	config_usart.pinmux_pad2 = PINMUX_UNUSED;
	config_usart.pinmux_pad3 = PINMUX_UNUSED;

	while (usart_init(&fingerprint_usart_instance, SERCOM0, &config_usart) != STATUS_OK);
	usart_enable(&fingerprint_usart_instance);
	
	//usart_register_callback(&fingerprint_usart_instance, fingerprint_read_callback, USART_CALLBACK_BUFFER_RECEIVED);
	//usart_enable_callback(&fingerprint_usart_instance, USART_CALLBACK_BUFFER_RECEIVED);	
	
	rx_semaphore = xSemaphoreCreateBinary();
}

void clear_usart_rx_buffer()
{
	uint8_t dummy;
	while (usart_read_wait(&fingerprint_usart_instance, &dummy) == STATUS_OK);
}


// Detecting finger and store the detected finger image in ImageBuffer
//int8_t gen_img() {
	//uint8_t cmd[] = GEN_IMG_CMD;
	//clear_usart_rx_buffer();
	//fingerprint_send_packet(cmd, sizeof(cmd));
	//vTaskDelay(pdMS_TO_TICKS(500));
	//
	//uint8_t ack[12];
	//fingerprint_read_response(ack, sizeof(ack));
	//
	//if(ack[9] == 0) {
		//LogMessage(LOG_INFO_LVL, "Finger collection success!\r\n");
		//return ack[9];
	//}
	//else {
		//LogMessage(LOG_ERROR_LVL, "Finger capture failed. Code: 0x%02X.\r\n", ack[9]);
		//return -1;
	//}
//}

int8_t gen_img() {
    uint8_t cmd[] = GEN_IMG_CMD;
    //clear_usart_rx_buffer();
    fingerprint_send_packet(cmd, sizeof(cmd));
    //vTaskDelay(pdMS_TO_TICKS(500));
    
    uint8_t ack[12];
    while(fingerprint_read_response(ack, sizeof(ack)) != STATUS_OK);
    
    if (ack[9] == 0) {
        LogMessage(LOG_INFO_LVL, "Finger collection success!\r\n");
        return 0;
    } else {
        LogMessage(LOG_ERROR_LVL, "Finger capture failed. Code: 0x%02X.\r\n", ack[9]);
        return -1;
    }
}



// Generate character file from the original finger image in ImageBuffer and store the file in CharBuffer1.
int8_t gen_cf_to_b1() {
	uint8_t cmd[] = GEN_CF_TO_B1_CMD;
	fingerprint_send_packet(cmd, sizeof(cmd));
	
	uint8_t ack[12];
	while(fingerprint_read_response(ack, sizeof(ack)) != STATUS_OK);
	
	if(ack[9] == 0) {
		LogMessage(LOG_INFO_LVL, "Character file 1 generated.\r\n");
		return ack[9];
	}
	else {
		LogMessage(LOG_ERROR_LVL, "Character file 1 generation failed. Code: 0x%02X.\r\n", ack[9]);
		return -1;
	}
}

// Generate character file from the original finger image in ImageBuffer and store the file in CharBuffer2.
int8_t gen_cf_to_b2() {
	uint8_t cmd[] = GEN_CF_TO_B2_CMD;
	fingerprint_send_packet(cmd, sizeof(cmd));
	
	uint8_t ack[12];
	while(fingerprint_read_response(ack, sizeof(ack)) != STATUS_OK);
	
	if(ack[9] == 0) {
		LogMessage(LOG_INFO_LVL, "Character file 2 generated.\r\n");
		return ack[9];
	}
	else {
		LogMessage(LOG_ERROR_LVL, "Character file 2 generation failed. Code: 0x%02X.\r\n", ack[9]);
		return -1;
	}
}

// combine information of character files from CharBuffer1 and CharBuffer2 and generate a template which is stored back in both CharBuffer1 and CharBuffer2
int8_t reg_model() {
	uint8_t cmd[] = REG_MODEL_CMD;
	fingerprint_send_packet(cmd, sizeof(cmd));
	
	uint8_t ack[12];
	while(fingerprint_read_response(ack, sizeof(ack)) != STATUS_OK);
	
	if(ack[9] == 0) {
		LogMessage(LOG_INFO_LVL, "Template successfully generated.\r\n");
		return ack[9];
	}
	else {
		LogMessage(LOG_ERROR_LVL, "Template generation failed. Code: 0x%02X.\r\n", ack[9]);
		return -1;
	}
}

// Store the template of specified buffer (Buffer1) at the designated location of Flash library
int8_t store_finger(uint8_t id) {
	uint8_t cmd[] = STORE_CMD(id);
	fingerprint_send_packet(cmd, sizeof(cmd));
	
	uint8_t ack[12];
	while(fingerprint_read_response(ack, sizeof(ack)) != STATUS_OK);
	
	if(ack[9] == 0) {
		LogMessage(LOG_INFO_LVL, "Stored fingerprint at ID %d.\r\n", id);
		return ack[9];
	}
	else {
		LogMessage(LOG_ERROR_LVL, "Store failed. Code: 0x%02X.\r\n", ack[9]);
		return -1;
	}
}

int find_smallest_index() {
	uint8_t cmd[] = READ_INDEX_CMD;
	fingerprint_send_packet(cmd, sizeof(cmd));

	uint8_t ack[44];
	while(fingerprint_read_response(ack, sizeof(ack)) != STATUS_OK);

	if (ack[9] == 0) {
		for (uint8_t byte = 0; byte < 32; byte++)
		{
			for (uint8_t bit = 0; bit < 8; bit++)
			{
				if (!((ack[10 + byte] >> bit) & 0x01))
				{
					LogMessage(LOG_INFO_LVL, "Smallest available index is: %d", byte * 8 + bit);
					return byte * 8 + bit;
				}
			}
		}
		
		return -1;
	}
	else {
		return -1;
	}
}

// Enroll fingerprint
int8_t fingerprint_enroll(uint8_t id) {
	CHECK_SUCCESS(gen_img());
    CHECK_SUCCESS(gen_cf_to_b1());
    CHECK_SUCCESS(gen_img());
    CHECK_SUCCESS(gen_cf_to_b2());
    CHECK_SUCCESS(reg_model());
    CHECK_SUCCESS(store_finger(id));
	
    return 0;
}

// Delete fingerprint
int8_t fingerprint_delete(uint8_t id) {	
	uint8_t cmd[] = DELETE_CMD(id);
	fingerprint_send_packet(cmd, sizeof(cmd));
	
	uint8_t ack[12];
	fingerprint_read_response(ack, sizeof(ack));
	
	if(ack[9] == 0) {
		LogMessage(LOG_INFO_LVL, "Fingerprint successfully deleted");
		return ack[9];
	}
	else {
		LogMessage(LOG_ERROR_LVL, "Fingerprint delete failed. Code: 0x%02X.\r\n", ack[9]);
		return -1;
	}
}

// Empty fingerprint library
int8_t fingerprint_empty() {
	uint8_t cmd[] = EMPTY_CMD;
	fingerprint_send_packet(cmd, sizeof(cmd));
	
	uint8_t ack[12];
	fingerprint_read_response(ack, sizeof(ack));
	
	if(ack[9] == 0) {
		LogMessage(LOG_INFO_LVL, "Fingerprint successfully emptied");
		return ack[9];
	}
	else {
		LogMessage(LOG_ERROR_LVL, "Fingerprint empty failed. Code: 0x%02X.\r\n", ack[9]);
		return -1;
	}
}

// Search fingerprint
int fingerprint_search() {
	CHECK_SUCCESS(gen_img());
	CHECK_SUCCESS(gen_cf_to_b1());
	
	// search fingerprint
	uint8_t cmd[] = SEARCH_CMD;
	fingerprint_send_packet(cmd, sizeof(cmd));
	
	uint8_t ack[16];
	while(fingerprint_read_response(ack, sizeof(ack)) != STATUS_OK);
	
	//vTaskDelay(pdMS_TO_TICKS(500));
	
	if(ack[9] == 0) {
		LogMessage(LOG_INFO_LVL, "Fingerprint matched. ID = %d.\r\n", ack[11]);
		return ack[11];
	}
	else {
		LogMessage(LOG_ERROR_LVL, "Search failed. Code: 0x%02X.\r\n.", ack[9]);
		return -1;
	}
}

// Read the number of fingers stored in library
int read_temp_num()
{
	uint8_t cmd[] = TEMP_NUM_CMD;
	fingerprint_send_packet(cmd, sizeof(cmd));

	uint8_t ack[14];
	fingerprint_read_response(ack, sizeof(ack));
	
	if(ack[9] == 0) {
		LogMessage(LOG_INFO_LVL, "Finger count: %d", ack[11]);
		return ack[11];
	}
	else {
		LogMessage(LOG_ERROR_LVL, "Read template count failed. Code: 0x%02X", ack[9]);
		return -1;
	}
}

/******************************************************************************
 * Local Helper Functions
 ******************************************************************************/
static void fingerprint_send_packet(uint8_t *packet, uint16_t length)
{
	int res = usart_write_buffer_wait(&fingerprint_usart_instance, packet, length);
}

static int fingerprint_read_response(uint8_t* buffer, uint16_t length)
{
	return usart_read_buffer_wait(&fingerprint_usart_instance, buffer, length);	
}

/******************************************************************************
 * Callback Function
 ******************************************************************************/
void fingerprint_read_callback(struct usart_module *const usart_module)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(rx_semaphore, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/******************************************************************************
 * Task Function
 ******************************************************************************/
void fingerprint_task(void *pvParameters){
	// init fingerprint module
	fingerprint_init();
	config_servo();
	
	while (1)
	{
		int finger_id = fingerprint_search();
		if (finger_id != -1)
		{
			cloud_send_finger_ID(finger_id);
			
			// unlock the door
			config_servo();
			pwm_set_servo_angle_unlock_door();
			vTaskDelay(pdMS_TO_TICKS(30000));
			pwm_set_servo_angle_lock_door();
		}
		vTaskDelay(pdMS_TO_TICKS(6000));
	}
}
