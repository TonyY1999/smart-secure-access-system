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

/******************************************************************************
 * Defines
 ******************************************************************************/

/******************************************************************************
 * Variables
 ******************************************************************************/
static struct usart_module fingerprint_usart_instance;

/******************************************************************************
 * Forward Declarations
 ******************************************************************************/
static void fingerprint_send_packet(uint8_t *packet, uint16_t length);
static void fingerprint_read_response(uint8_t *buffer, uint16_t length);

/******************************************************************************
 * Global Functions
 ******************************************************************************/
void fingerprint_init()
{
	struct usart_config config_usart;
	usart_get_config_defaults(&config_usart);

	config_usart.baudrate    = 9600;
	config_usart.mux_setting = USART_RX_1_TX_0_XCK_1;
	config_usart.pinmux_pad0 = PINMUX_PA16C_SERCOM1_PAD0;  // TX
	config_usart.pinmux_pad1 = PINMUX_PA17C_SERCOM1_PAD1;  // RX
	config_usart.pinmux_pad2 = PINMUX_UNUSED; 
	config_usart.pinmux_pad3 = PINMUX_UNUSED; 

	while (usart_init(&fingerprint_usart_instance, SERCOM1, &config_usart) != STATUS_OK);
	usart_enable(&fingerprint_usart_instance);
}

// Detecting finger and store the detected finger image in ImageBuffer
uint8_t gen_img() {
	uint8_t cmd[] = GEN_IMG_CMD;
	fingerprint_send_packet(cmd, sizeof(cmd));
	
	uint8_t ack[12];
	fingerprint_read_response(ack, sizeof(ack));
	
	if(ack[9] == 0) {
		LogMessage(LOG_INFO_LVL, "Finger collection success!\r\n");
	}
	else {
		LogMessage(LOG_ERROR_LVL, "Finger capture failed. Code: 0x%02X.\r\n", ack[9]);
	}
	
	return ack[9];
}

// Generate character file from the original finger image in ImageBuffer and
// store the file in CharBuffer1.
uint8_t gen_cf_to_b1() {
	uint8_t cmd[] = GEN_CF_TO_B1_CMD;
	fingerprint_send_packet(cmd, sizeof(cmd));
	
	uint8_t ack[12];
	fingerprint_read_response(ack, sizeof(ack));
	
	if(ack[9] == 0) {
		LogMessage(LOG_INFO_LVL, "CharBuffer1 generated.\r\n");
	}
	else {
		LogMessage(LOG_ERROR_LVL, "CharBuffer1 failed. Code: 0x%02X.\r\n", ack[9]);
	}
	
	return ack[9];
}

// Generate character file from the original finger image in ImageBuffer and
// store the file in CharBuffer2.
void gen_cf_to_b2() {
	uint8_t cmd[] = GEN_CF_TO_B2_CMD;
	fingerprint_send_packet(cmd, sizeof(cmd));
	
	uint8_t ack[12];
	fingerprint_read_response(ack, sizeof(ack));
	
	if(ack[9] == 0) {
		LogMessage(LOG_INFO_LVL, "CharBuffer2 generated.\r\n");
	}
	else {
		LogMessage(LOG_ERROR_LVL, "CharBuffer2 failed. Code: 0x%02X.\r\n", ack[9]);
	}
}

// combine information of character files from CharBuffer1 and CharBuffer2
// and generate a template which is stored back in both CharBuffer1 and CharBuffer2
void reg_model() {
	uint8_t cmd[] = REG_MODEL_CMD;
	fingerprint_send_packet(cmd, sizeof(cmd));
	
	uint8_t ack[12];
	fingerprint_read_response(ack, 12);
	
	if(ack[9] == 0) {
		LogMessage(LOG_INFO_LVL, "Template successfully generated.\r\n");
	}
	else {
		LogMessage(LOG_ERROR_LVL, "Template generation failed. Code: 0x%02X.\r\n", ack[9]);
	}
}

// Store the template of specified buffer (Buffer1) at the designated location of Flash library
void store_finger(uint8_t id) {
	uint8_t cmd[] = STORE_CMD(id);
	fingerprint_send_packet(cmd, sizeof(cmd));
	
	uint8_t ack[12];
	fingerprint_read_response(ack, 12);
	
	if(ack[9] == 0) {
		LogMessage(LOG_INFO_LVL, "Stored fingerprint at ID %d.\r\n", id);
	}
	else {
		LogMessage(LOG_ERROR_LVL, "Store failed. Code: 0x%02X.\r\n", ack[9]);
	}
}

// Enroll fingerprint
void fingerprint_enroll(uint8_t id) {
	// Collect finger
	LogMessage(LOG_INFO_LVL, "Place finger on sensor...\r\n");
	vTaskDelay(pdMS_TO_TICKS(3000));
	gen_img();
	
	// Generate character file
	gen_cf_to_b1();
	SerialConsoleWriteString("Remove your finger.\r\n");
	vTaskDelay(pdMS_TO_TICKS(3000));
	
	// Collect finger
	SerialConsoleWriteString("Put your finger on sensor....\r\n");
	vTaskDelay(pdMS_TO_TICKS(3000));
	gen_img();
	
	// Generate character file
	gen_cf_to_b2();
	SerialConsoleWriteString("Remove your finger.\r\n");
	vTaskDelay(pdMS_TO_TICKS(3000));
	
	// Combine to template
	reg_model();
	
	// Store finger to library
	store_finger(id);
}

// Recognize fingerprint
uint8_t fingerprint_search() {
	// Collect finger
	LogMessage(LOG_INFO_LVL, "Place finger on sensor...\r\n");
	vTaskDelay(pdMS_TO_TICKS(3000));
	gen_img();
		
	// Generate character file
	gen_cf_to_b1();
	SerialConsoleWriteString("Remove your finger.\r\n");
	vTaskDelay(pdMS_TO_TICKS(3000));
		
	// Collect finger
	SerialConsoleWriteString("Put your finger on sensor....\r\n");
	vTaskDelay(pdMS_TO_TICKS(3000));
	gen_img();
		
	// Generate character file
	gen_cf_to_b2();
	SerialConsoleWriteString("Remove your finger.\r\n");
	vTaskDelay(pdMS_TO_TICKS(3000));
		
	// Combine to template
	reg_model();
	
	// Search fingerprint
	uint8_t cmd[] = SEARCH_CMD;
	fingerprint_send_packet(cmd, sizeof(cmd));
	
	uint8_t ack[16];
	fingerprint_send_packet(ack, sizeof(ack));
	
	if(ack[9] == 0) {
        LogMessage(LOG_INFO_LVL, "Fingerprint matched. ID = %d.\r\n", ack[11]);
        return ack[11];
	}
	else if(ack[9] == 1) {
		LogMessage(LOG_ERROR_LVL, "Error when receiving package.\r\n");
		return 0xFE;
	}
	else {
        LogMessage(LOG_WARNING_LVL, "No match found. Code: 0x%02X.\r\n", ack[9]);
        return 0xFE;
	}
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
	//fingerprint_read_response(ack, sizeof(ack));
	
	for (int i = 0; i < 12; i++) {
		usart_read_wait(&fingerprint_usart_instance, &ack[i]);
		LogMessage(LOG_INFO_LVL, "jj: %d.\r\n", ack[i]);
	}
	
	if (ack[9] == 0)
	{
		SerialConsoleWriteString("1");
		LogMessage(LOG_INFO_LVL, "Parameter setting complete.\r\n");
	}
	else {
		SerialConsoleWriteString("2");
		LogMessage(LOG_ERROR_LVL, "Parameter setting failed, code: 0x%02X.\r\n", ack[9]);
	}
}

// Read fingerprint sensor system parameters
void read_sys_para()
{
	uint8_t command_packet[] = READ_SYS_CMD;
	fingerprint_send_packet(command_packet, sizeof(command_packet));

	uint8_t response_packet[28];
	fingerprint_read_response(response_packet, 28);
}

// Read the number of fingers stored in library
void read_temp_num()
{
	uint8_t cmd[] = TEMP_NUM_CMD;
	fingerprint_send_packet(cmd, sizeof(cmd));

	uint8_t ack[14];
	fingerprint_read_response(ack, sizeof(ack));
	
	if(ack[9] == 0) {
		LogMessage(LOG_INFO_LVL, "Finger count: %d", ack[11]);
	}
	else {
		LogMessage(LOG_ERROR_LVL, "Read template count failed. Code: 0x%02X", ack[9]);
	}
}

/******************************************************************************
 * Local Helper Functions
 ******************************************************************************/
static void fingerprint_send_packet(uint8_t *packet, uint16_t length)
{
	usart_write_buffer_wait(&fingerprint_usart_instance, packet, length);
}

static void fingerprint_read_response(uint8_t *buffer, uint16_t length)
{
	usart_read_buffer_wait(&fingerprint_usart_instance, buffer, length);
}