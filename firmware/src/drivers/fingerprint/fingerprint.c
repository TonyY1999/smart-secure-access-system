/**************************************************************************//**
 * @file      fingerprint_driver.c
 * @brief     Source file for fingerprint driver functions in the Smart Secure Access System.
 * @author    Tony Yan & Yue Zhang
 * @date      2026-08-02
 ******************************************************************************/

/******************************************************************************
 * Includes
 ******************************************************************************/
#include "fingerprint.h"
#include "usart.h"

/******************************************************************************
 * Defines
 ******************************************************************************/
// Fingerprint module USART configuration
#define FP_SERCOM              SERCOM0
#define FP_SERCOM_MUX_SETTING  USART_RX_1_TX_0_XCK_1

#define FP_PINMUX_PAD0         PINMUX_PA08C_SERCOM0_PAD0  // FP_TX
#define FP_PINMUX_PAD1         PINMUX_PA09C_SERCOM0_PAD1  // FP_RX
#define FP_PINMUX_PAD2         PINMUX_UNUSED
#define FP_PINMUX_PAD3         PINMUX_UNUSED

#define FP_BAUDRATE 9600

// Fingerprint data packet fixed header and address
#define FP_HEADER   0xEF01u
#define FP_ADDRESS  0xFFFFFFFFu

// Fingerprint data packet identifiers(PID)
#define FP_PID_COMMAND      0x01u
#define FP_PID_ACK          0x07u

// Fingerprint data packet command contents(instruction codes)
#define FP_CMD_GET_IMAGE           0x01u
#define FP_CMD_IMAGE_TO_CHAR       0x02u
#define FP_CMD_MATCH               0x03u
#define FP_CMD_SEARCH              0x04u
#define FP_CMD_CREATE_MODEL        0x05u
#define FP_CMD_STORE_MODEL         0x06u

#define FP_CMD_UP_IMAGE            0x0Au

#define FP_CMD_DELETE_CHAR         0x0Cu
#define FP_CMD_EMPTY_LIBRARY       0x0Du

#define FP_CMD_TEMPLATE_COUNT      0x1Du

#define FP_CMD_GET_IMAGE_EX        0x28u

#define FP_CMD_LED_CONTROL         0x35u
#define FP_CMD_CHECK_SENSOR        0x36u

#define FP_CMD_HANDSHAKE           0x40u

// Fingerprint data packet maximum sizes(unit in bytes)
#define FP_CMD_PACKET_MAX_SIZE     32u
#define FP_ACK_PACKET_MAX_SIZE     32u

/******************************************************************************
 * Variables
 ******************************************************************************/
static struct usart_module fp_usart_instance;  // USART instance for fingerprint sensor communication

/******************************************************************************
 * Forward Declarations
 ******************************************************************************/
// Helper functions to convert between byte arrays and integers
static void fp_put_u16(uint8_t* buf, uint16_t value);

// Helper functions to convert between byte arrays and integers
static void fp_put_u32(uint8_t* buf, uint32_t value);

// Helper function to convert a byte array to a 16-bit integer
static uint16_t fp_get_u16(const uint8_t* buf);

// Helper function to convert a byte array to a 32-bit integer
static uint32_t fp_get_u32(const uint8_t *buf);

/**
 * @brief Send a command packet to the fingerprint sensor.
 *
 * @param[in] content Pointer to the command content to be sent.
 * @param[in] content_len Length of the command content in bytes.
 *
 * @return fp_status_t structure containing the status.
 */
static fp_status_t fp_send_cmd(const uint8_t* content, uint16_t content_len);

/**
 * @brief Read the acknowledgment packet from the fingerprint sensor.
 *
 * @param[in] None
 * @param[out] ack Pointer to the buffer where the acknowledgment packet will be stored.
 *
 * @return fp_status_t structure containing the status.
 */
static fp_status_t fp_read_ack(uint8_t* ack);

/******************************************************************************
 * Global Functions
 ******************************************************************************/
bool fingerprint_init(void)
{	
	struct usart_config fp_usart_config;
	usart_get_config_defaults(&fp_usart_config);
    
    // Configure USART settings for fingerprint sensor communication
	fp_usart_config.baudrate = FP_BAUDRATE;
	fp_usart_config.mux_setting = FP_SERCOM_MUX_SETTING;
	fp_usart_config.pinmux_pad0 = FP_PINMUX_PAD0;  // FP_TX
	fp_usart_config.pinmux_pad1 = FP_PINMUX_PAD1;  // FP_RX
	fp_usart_config.pinmux_pad2 = FP_PINMUX_PAD2;
	fp_usart_config.pinmux_pad3 = FP_PINMUX_PAD3;

	if(usart_init(&fp_usart_instance, FP_SERCOM, &fp_usart_config) != STATUS_OK) {
		return false;
	}

	usart_enable(&fp_usart_instance);

	delay_ms(200); // Wait for the sensor to power up and be ready for communication
    
	return true;
}

fp_result_t get_img(void)
{
    // initialize the result structure with default values
    fp_result_t result = {FP_STATUS_OK, FP_INVALID_CONFIRMATION_CODE};
    
    uint8_t content[] = {FP_CMD_GET_IMAGE};

    // send the command to the fingerprint sensor
    fp_status_t send_status = fp_send_cmd(content, sizeof(content));
    if (send_status != FP_STATUS_OK) {
        result.status = send_status;
        return result;
    }

    // read the acknowledgment packet from the fingerprint sensor
    uint8_t ack[FP_ACK_PACKET_MAX_SIZE];
    fp_status_t read_status = fp_read_ack(ack);
    if(read_status != FP_STATUS_OK) {
        result.status = read_status;
        return result;
    }

    // store the confirmation code from the acknowledgment packet
    result.confirmation_code = ack[9];

    return result;
}

fp_result_t img_to_char(fp_buffer_id_t buffer_id)
{
    // initialize the result structure with default values
    fp_result_t result = {FP_STATUS_OK, FP_INVALID_CONFIRMATION_CODE};

    uint8_t content[] = {FP_CMD_IMAGE_TO_CHAR, (uint8_t)buffer_id};

    // send the command to the fingerprint sensor
    fp_status_t send_status = fp_send_cmd(content, sizeof(content));
    if (send_status != FP_STATUS_OK) {
        result.status = send_status;
        return result;
    }

    // read the acknowledgment packet from the fingerprint sensor
    uint8_t ack[FP_ACK_PACKET_MAX_SIZE];
    fp_status_t read_status = fp_read_ack(ack);
    if(read_status != FP_STATUS_OK) {
        result.status = read_status;
        return result;
    }

    // store the confirmation code from the acknowledgment packet
    result.confirmation_code = ack[9];

    return result;
}

fp_result_t create_model(void)
{
    // initialize the result structure with default values
    fp_result_t result = {FP_STATUS_OK, FP_INVALID_CONFIRMATION_CODE};

    uint8_t content[] = {FP_CMD_CREATE_MODEL};

    // send the command to the fingerprint sensor
    fp_status_t send_status = fp_send_cmd(content, sizeof(content));
    if (send_status != FP_STATUS_OK) {
        result.status = send_status;
        return result;
    }

    // read the acknowledgment packet from the fingerprint sensor
    uint8_t ack[FP_ACK_PACKET_MAX_SIZE];
    fp_status_t read_status = fp_read_ack(ack);
    if(read_status != FP_STATUS_OK) {
        result.status = read_status;
        return result;
    }

    // store the confirmation code from the acknowledgment packet
    result.confirmation_code = ack[9];

    return result;
}   

fp_result_t store_model(uint16_t fp_id)
{
    // initialize the result structure with default values
    fp_result_t result = {FP_STATUS_OK, FP_INVALID_CONFIRMATION_CODE};

    // Command + Buffer ID + Fingerprint ID (2 bytes)
    uint8_t content[] = {FP_CMD_STORE_MODEL, FP_CHAR_BUFFER_1, fp_id >> 8, fp_id & 0xFF}; 

    // send the command to the fingerprint sensor
    fp_status_t send_status = fp_send_cmd(content, sizeof(content));
    if (send_status != FP_STATUS_OK) {
        result.status = send_status;
        return result;
    }

    // read the acknowledgment packet from the fingerprint sensor
    uint8_t ack[FP_ACK_PACKET_MAX_SIZE];
    fp_status_t read_status = fp_read_ack(ack);
    if(read_status != FP_STATUS_OK) {
        result.status = read_status;
        return result;
    }

    // store the confirmation code from the acknowledgment packet
    result.confirmation_code = ack[9];

    return result;
}

fp_result_t fp_empty_library(void)
{
    // initialize the result structure with default values
    fp_result_t result = {FP_STATUS_OK, FP_INVALID_CONFIRMATION_CODE};

    uint8_t content[] = {FP_CMD_EMPTY_LIBRARY};

    // send the command to the fingerprint sensor
    fp_status_t send_status = fp_send_cmd(content, sizeof(content));
    if (send_status != FP_STATUS_OK) {
        result.status = send_status;
        return result;
    }

    // read the acknowledgment packet from the fingerprint sensor
    uint8_t ack[FP_ACK_PACKET_MAX_SIZE];
    fp_status_t read_status = fp_read_ack(ack);
    if(read_status != FP_STATUS_OK) {
        result.status = read_status;
        return result;
    }

    // store the confirmation code from the acknowledgment packet
    result.confirmation_code = ack[9];

    return result;
}

fp_result_t fp_read_temp_num(uint16_t* temp_nums)
{
    // initialize the result structure with default values
    fp_result_t result = {FP_STATUS_OK, FP_INVALID_CONFIRMATION_CODE};

    uint8_t content[] = {FP_CMD_TEMPLATE_COUNT};

    // send the command to the fingerprint sensor
    fp_status_t send_status = fp_send_cmd(content, sizeof(content));
    if (send_status != FP_STATUS_OK) {
        result.status = send_status;
        return result;
    }

    // read the acknowledgment packet from the fingerprint sensor
    uint8_t ack[FP_ACK_PACKET_MAX_SIZE];
    fp_status_t read_status = fp_read_ack(ack);
    if(read_status != FP_STATUS_OK) {
        result.status = read_status;
        return result;
    }

    // store the confirmation code from the acknowledgment packet
    result.confirmation_code = ack[9];

    // Only parse template number when command succeeded
    if(result.confirmation_code == 0x00u) {
        *temp_nums = get_u16(&ack[10]);
    }

    return result;
}


/******************************************************************************
 * Local Helper Functions
 ******************************************************************************/
static void fp_put_u16(uint8_t *buf, uint16_t value) 
{
    buf[0] = (uint8_t)(value >> 8);
    buf[1] = (uint8_t)(value & 0xFF);
}

static void fp_put_u32(uint8_t *buf, uint32_t value) 
{
    buf[0] = (uint8_t)(value >> 24);
    buf[1] = (uint8_t)(value >> 16);
    buf[2] = (uint8_t)(value >> 8);
    buf[3] = (uint8_t)(value & 0xFF);
}

static uint16_t fp_get_u16(const uint8_t *buf) 
{
    return ((uint16_t)buf[0] << 8) | buf[1];
}

static uint32_t fp_get_u32(const uint8_t *buf) 
{
    return ((uint32_t)buf[0] << 24) | ((uint32_t)buf[1] << 16) | ((uint32_t)buf[2] << 8) | buf[3];
}

static fp_status_t fp_send_cmd(const uint8_t* content, uint16_t content_len)
{
    // Check if the content pointer is valid
    if(content == NULL) {
        return FP_ERR_INVALID_CONTENT;
    }

    // Check if the content length is valid
    if(content_len == 0 || (content_len + 11u) > FP_CMD_PACKET_MAX_SIZE) {
        return FP_ERR_INVALID_CONTENT_LENGTH;
    }

    // create a buffer to hold the entire packet to be sent
    uint8_t cmd[FP_CMD_PACKET_MAX_SIZE];
    uint16_t checksum = 0;
    uint16_t index = 0;

    // build the header(2 bytes)
    fp_put_u16(&cmd[index], FP_HEADER);
    index += 2;

    // build the address(4 bytes)
    fp_put_u32(&cmd[index],FP_ADDRESS);
    index += 4;

    // build the PID(packet identifier)(1 byte)
    cmd[index++] = FP_PID_COMMAND;
    checksum += (uint16_t)FP_PID_COMMAND;

    // build the length field
    // length = content_len + checksum(2 bytes)
    uint16_t length = (uint16_t)(content_len + 2);
    fp_put_u16(&cmd[index], length);
    index += 2;
    checksum += (uint16_t)(length >> 8);
    checksum += (uint16_t)(length & 0xFF);

    // build the command and parameters
    for(uint16_t i = 0; i < content_len; i++)
    {
        cmd[index++] = content[i];
        checksum += (uint16_t)content[i];
    }

    // build the checksum
    fp_put_u16(&cmd[index], checksum);
    index += 2;
    
    // UART send the packet
    if(usart_write_buffer_wait(&fp_usart_instance, cmd, index) != STATUS_OK)
    {
        return FP_ERR_UART_SEND;
    }

    return FP_STATUS_OK;
}

static fp_status_t fp_read_ack(uint8_t* ack)
{
    // Check if the ack pointer is valid
    if(ack == NULL) {
        return FP_ERR_INVALID_CONTENT;
    }

    // read the first 9 bytes of the acknowledgment packet from the fingerprint sensor
    if (usart_read_buffer_wait(&fp_usart_instance, ack, 9u) != STATUS_OK) {
        return FP_ERR_UART_RECEIVE;
    }

    // check if the acknowledgment packet header is valid
    if((fp_get_u16(&ack[0])) != FP_HEADER) {
        return FP_ERR_HEADER;
    }

    // check if the acknowledgment packet address is valid
    if((fp_get_u32(&ack[2])) != FP_ADDRESS) {
        return FP_ERR_ADDRESS;
    }

    uint16_t package_length = fp_get_u16(&ack[7]);

    // read the remaining bytes of the acknowledgment packet from the fingerprint sensor
    if (usart_read_buffer_wait(&fp_usart_instance, &ack[9], package_length) != STATUS_OK) {
        return FP_ERR_UART_RECEIVE;
    }

    // check if the acknowledgment packets correct
    uint16_t total_length = package_length + 9u;
    uint16_t received_checksum = 0;
    for(uint16_t i = 6u; i < total_length - 2u; i++) {
        received_checksum += (uint16_t)ack[i];
    }

    if(received_checksum != (fp_get_u16(&ack[total_length - 2u]))) {
        return FP_ERR_CHECKSUM;
    }

    return FP_STATUS_OK;
}
