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
#include "SerialConsole.h"
#include <asf.h>

/******************************************************************************
 * Defines
 ******************************************************************************/
#define ACK_BUFFER_SIZE 32

/******************************************************************************
 * Variables
 ******************************************************************************/
static struct usart_module fingerprint_usart_instance;

static SemaphoreHandle_t rx_semaphore;

static volatile uint8_t latest_rx;
static uint8_t rx_buffer[ACK_BUFFER_SIZE];
static uint16_t rx_index = 0;
static uint16_t rx_expected_len = 0;

extern TaskHandle_t servoTaskHandle;

/******************************************************************************
 * Forward Declarations
 ******************************************************************************/
static void fingerprint_send_packet(uint8_t *packet, uint16_t length);
static void fingerprint_read_response(uint8_t *buffer, uint16_t length);

static void fingerprint_read_callback(struct usart_module *const usart_module);

/******************************************************************************
 * Global Functions
 ******************************************************************************/
void fingerprint_init()
{	
	struct usart_config config_usart;
	usart_get_config_defaults(&config_usart);

	config_usart.baudrate = 9600;
	config_usart.character_size = USART_CHARACTER_SIZE_8BIT;
	config_usart.parity = USART_PARITY_NONE;
	config_usart.stopbits = USART_STOPBITS_1;
	
	config_usart.mux_setting = USART_RX_1_TX_0_XCK_1;
	config_usart.pinmux_pad0 = PINMUX_PA08C_SERCOM0_PAD0;  // TX
	config_usart.pinmux_pad1 = PINMUX_PA09C_SERCOM0_PAD1;  // RX
	config_usart.pinmux_pad2 = PINMUX_UNUSED;
	config_usart.pinmux_pad3 = PINMUX_UNUSED;

	while (usart_init(&fingerprint_usart_instance, SERCOM0, &config_usart) != STATUS_OK);
	usart_enable(&fingerprint_usart_instance);
		
	usart_register_callback(&fingerprint_usart_instance, fingerprint_read_callback, USART_CALLBACK_BUFFER_RECEIVED);
	usart_enable_callback(&fingerprint_usart_instance, USART_CALLBACK_BUFFER_RECEIVED);	
	
	rx_semaphore = xSemaphoreCreateBinary();
}

// Detecting finger and store the detected finger image in ImageBuffer
uint8_t gen_img() {
	uint8_t cmd[] = GEN_IMG_CMD;
	fingerprint_send_packet(cmd, sizeof(cmd));
	
	uint8_t ack[12];
	fingerprint_read_response(ack, sizeof(ack));
	
	//if(ack[9] == 0) {
		//LogMessage(LOG_INFO_LVL, "Finger collection success!\r\n");
	//}
	//else {
		//LogMessage(LOG_ERROR_LVL, "Finger capture failed. Code: 0x%02X.\r\n", ack[9]);
	//}
	
	return ack[9];
}

// Generate character file from the original finger image in ImageBuffer and
// store the file in CharBuffer1.
uint8_t gen_cf_to_b1() {
	uint8_t cmd[] = GEN_CF_TO_B1_CMD;
	fingerprint_send_packet(cmd, sizeof(cmd));
	
	uint8_t ack[12];
	fingerprint_read_response(ack, sizeof(ack));
	
	//if(ack[9] == 0) {
		//LogMessage(LOG_INFO_LVL, "CharBuffer1 generated.\r\n");
	//}
	//else {
		//LogMessage(LOG_ERROR_LVL, "CharBuffer1 failed. Code: 0x%02X.\r\n", ack[9]);
	//}
	
	return ack[9];
}

// Generate character file from the original finger image in ImageBuffer and
// store the file in CharBuffer2.
uint8_t gen_cf_to_b2() {
	uint8_t cmd[] = GEN_CF_TO_B2_CMD;
	fingerprint_send_packet(cmd, sizeof(cmd));
	
	uint8_t ack[12];
	fingerprint_read_response(ack, sizeof(ack));
	
	//if(ack[9] == 0) {
		//LogMessage(LOG_INFO_LVL, "CharBuffer2 generated.\r\n");
	//}
	//else {
		//LogMessage(LOG_ERROR_LVL, "CharBuffer2 failed. Code: 0x%02X.\r\n", ack[9]);
	//}
	return ack[9];
}

// combine information of character files from CharBuffer1 and CharBuffer2
// and generate a template which is stored back in both CharBuffer1 and CharBuffer2
uint8_t reg_model() {
	uint8_t cmd[] = REG_MODEL_CMD;
	fingerprint_send_packet(cmd, sizeof(cmd));
	
	uint8_t ack[12];
	fingerprint_read_response(ack, 12);
	
	//if(ack[9] == 0) {
		//LogMessage(LOG_INFO_LVL, "Template successfully generated.\r\n");
	//}
	//else {
		//LogMessage(LOG_ERROR_LVL, "Template generation failed. Code: 0x%02X.\r\n", ack[9]);
	//}
	
	return ack[9];
}

// Store the template of specified buffer (Buffer1) at the designated location of Flash library
uint8_t store_finger(uint8_t id) {
	uint8_t cmd[] = STORE_CMD(id);
	fingerprint_send_packet(cmd, sizeof(cmd));
	
	uint8_t ack[12];
	fingerprint_read_response(ack, sizeof(ack));
	
	//if(ack[9] == 0) {
		//LogMessage(LOG_INFO_LVL, "Stored fingerprint at ID %d.\r\n", id);
	//}
	//else {
		//LogMessage(LOG_ERROR_LVL, "Store failed. Code: 0x%02X.\r\n", ack[9]);
	//}
	
	return ack[9];
}

// Enroll fingerprint
void fingerprint_enroll(uint8_t id) {
	// Collect finger
	//SerialConsoleWriteString("Place finger on sensor...\r\n");
	//vTaskDelay(pdMS_TO_TICKS(3000));
	gen_img();
	
	//vTaskDelay(pdMS_TO_TICKS(100));
	
	// Generate character file
	gen_cf_to_b1();
	//SerialConsoleWriteString("Remove your finger.\r\n");
	//vTaskDelay(pdMS_TO_TICKS(3000));
	
	// Collect finger
	//SerialConsoleWriteString("Put your finger on sensor....\r\n");
	//vTaskDelay(pdMS_TO_TICKS(3000));
	gen_img();
	
	// Generate character file
	gen_cf_to_b2();
	//SerialConsoleWriteString("Remove your finger.\r\n");
	//vTaskDelay(pdMS_TO_TICKS(3000));
	
	// Combine to template
	reg_model();
	
	// Store finger to library
	int res = store_finger(id);
}

// Recognize fingerprint
uint8_t fingerprint_search() {
	// Collect finger
	//LogMessage(LOG_INFO_LVL, "Place finger on sensor...\r\n");
	//vTaskDelay(pdMS_TO_TICKS(3000));
	gen_img();
		
	// Generate character file
	gen_cf_to_b1();
	//SerialConsoleWriteString("Remove your finger.\r\n");
	//vTaskDelay(pdMS_TO_TICKS(3000));
		//
	// Collect finger
	//SerialConsoleWriteString("Put your finger on sensor....\r\n");
	//vTaskDelay(pdMS_TO_TICKS(3000));
	gen_img();
		
	// Generate character file
	gen_cf_to_b2();
	//SerialConsoleWriteString("Remove your finger.\r\n");
	//vTaskDelay(pdMS_TO_TICKS(3000));
		
	// Combine to template
	reg_model();
	
	// Search fingerprint
	uint8_t cmd[] = SEARCH_CMD;
	fingerprint_send_packet(cmd, sizeof(cmd));
	
	uint8_t ack[16];
	fingerprint_read_response(ack, sizeof(ack));
	//
	//if(ack[9] == 0) {
        //LogMessage(LOG_INFO_LVL, "Fingerprint matched. ID = %d.\r\n", ack[11]);
        //return ack[11];
	//}
	//else if(ack[9] == 1) {
		//LogMessage(LOG_ERROR_LVL, "Error when receiving package.\r\n");
		//return 0xFE;
	//}
	//else {
        //LogMessage(LOG_WARNING_LVL, "No match found. Code: 0x%02X.\r\n", ack[9]);
        //return 0xFE;
	//}
	
	return ack[9];
}

// Set the fingerprint sensor baud rate to 9600
void set_baud_rate_9600()
{
	uint8_t cmd[] = {
		0xEF, 0x01,             // Header
		0xFF, 0xFF, 0xFF, 0xFF, // Address
		0x01,                   // Package ID (Command packet)
		0x00, 0x05,             // Package length (5 bytes)
		0x0E,                   // Instruction code (SetSysPara)
		0x04,                   // Parameter number (Baud rate)
		0x01,                   // New baud rate (e.g., 1 for 9600bps)
		0x00, 0x19
	};

	fingerprint_send_packet(cmd, 14);

	uint8_t ack[12];
	fingerprint_read_response(ack, sizeof(ack));
	
	if (ack[9] == 0)
	{
		LogMessage(LOG_INFO_LVL, "Parameter setting complete.\r\n");
	}
	else {
		LogMessage(LOG_ERROR_LVL, "Parameter setting failed, code: 0x%02X.\r\n", ack[9]);
	}
}

// Read fingerprint sensor system parameters
void read_sys_para()
{
	uint8_t cmd[] = READ_SYS_CMD;
	fingerprint_send_packet(cmd, sizeof(cmd));
	
	uint8_t ack[28];
	fingerprint_read_response(ack, sizeof(ack));
}

// Read the number of fingers stored in library
void read_temp_num()
{
	uint8_t cmd[] = TEMP_NUM_CMD;
	fingerprint_send_packet(cmd, sizeof(cmd));

	uint8_t ack[14];
	fingerprint_read_response(ack, sizeof(ack));
	//if(ack[9] == 0) {
		//LogMessage(LOG_INFO_LVL, "Finger count: %d", ack[11]);
	//}
	//else {
		//LogMessage(LOG_ERROR_LVL, "Read template count failed. Code: 0x%02X", ack[9]);
	//}
}

/******************************************************************************
 * Local Helper Functions
 ******************************************************************************/
static void fingerprint_send_packet(uint8_t *packet, uint16_t length)
{
	int res = usart_write_buffer_wait(&fingerprint_usart_instance, packet, length);
}

static void fingerprint_read_response(uint8_t* buffer, uint16_t length)
{
	while (usart_get_job_status(&fingerprint_usart_instance, USART_TRANSCEIVER_RX) == STATUS_BUSY) {
		vTaskDelay(1);
	}

	if (usart_read_buffer_job(&fingerprint_usart_instance, buffer, length) != STATUS_OK) {
		LogMessage(LOG_ERROR_LVL, "USART RX job failed\r\n");
		return;
	}

	if (xSemaphoreTake(rx_semaphore, pdMS_TO_TICKS(1000)) != pdTRUE) {
		LogMessage(LOG_ERROR_LVL, "Timeout waiting for USART RX\r\n");
	}
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
	fingerprint_init();
	
	
	//fingerprint_enroll(3);
	//read_temp_num();
	
	//fingerprint_search();
	
	while (1)
	{
		if (fingerprint_search() == 0)
		{
			vTaskResume(servoTaskHandle);
		}
		
		vTaskDelay(100);
	}
}
