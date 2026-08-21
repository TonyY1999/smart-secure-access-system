/**************************************************************************//**
 * @file      fingerprint_driver.c
 * @brief     Source file for fingerprint driver functions in the Smart Secure Access System.
 * @author    Tony Yan & Yue Zhang
 * @date      2026-08-02
 ******************************************************************************/

/******************************************************************************
 * Includes
 ******************************************************************************/
#include "fingerprint_driver.h"
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

// A fingerprint data packet consists of a fixed header, address, PID, length, content, and checksum.
// Header(2 bytes) + Address(4 bytes) + PID(1 byte) + Length(2 bytes) + Content(N bytes) + Checksum(2 bytes)

// Header: Fixed value of 0xEF01; High byte transferred first.
// Address: Fixed value of 0xFFFFFFFF; High byte transferred first.
// PID: Packet identifier; 0x01 for command packets, 0x07 for acknowledgment packets.
// Length: length of package content plus the length of Checksum( 2 bytes), unit is byte. High byte transferred first.
// Content: Command or data specific to the packet type; variable length.
// Checksum: Sum of all bytes in the packet, excluding the header and checksum itself; High byte transferred first.

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
#define FP_TX_PACKET_MAX_SIZE      32u
#define FP_ACK_DATA_MAX_SIZE       32u

/******************************************************************************
 * Variables
 ******************************************************************************/
static struct usart_module fp_usart_instance;  // USART instance for fingerprint sensor communication

/******************************************************************************
 * Forward Declarations
 ******************************************************************************/
// Helper functions to convert between byte arrays and integers
static void fp_put_u16(uint8_t *buf, uint16_t value);

// Helper functions to convert between byte arrays and integers
static void fp_put_u32(uint8_t *buf, uint32_t value);

// Helper function to convert a byte array to a 16-bit integer
static uint16_t fp_get_u16(const uint8_t *buf);

// send command to fingerprint sensor
static bool fp_send_command(const uint8_t *content, uint16_t content_len);

static bool fp_read_ack(void);

/******************************************************************************
 * Global Functions
 ******************************************************************************/
bool fingerprint_init(void)
{	
	struct usart_config usart_config_instance;
	usart_get_config_defaults(&usart_config_instance);
    
    // Configure USART settings for fingerprint sensor communication
	usart_config_instance.baudrate = FP_BAUDRATE;
	usart_config_instance.mux_setting = FP_SERCOM_MUX_SETTING;
	usart_config_instance.pinmux_pad0 = FP_PINMUX_PAD0;  // FP_TX
	usart_config_instance.pinmux_pad1 = FP_PINMUX_PAD1;  // FP_RX
	usart_config_instance.pinmux_pad2 = FP_PINMUX_PAD2;
	usart_config_instance.pinmux_pad3 = FP_PINMUX_PAD3;

	if(usart_init(&fp_usart_instance, FP_SERCOM, &usart_config_instance) != STATUS_OK) {
		return false;
	}

	usart_enable(&fp_usart_instance);

	delay_ms(200); // Wait for the sensor to power up and be ready for communication
    
	return true;
}

fp_result_t get_img(void)
{
    uint8_t instruction_code[] = {FP_CMD_GET_IMAGE};

    fp_result_t result = fp_send_command(instruction_code, sizeof(instruction_code));
    if (result.status != FP_OK) {
        return result;
    }
    
    return fp_read_ack();
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


static fp_result_t fp_send_command(const uint8_t *content, uint16_t content_len)
{
    fp_result_t result = {0};

    // validate input parameters
    if(content == NULL || content_len == 0) {
        result.status = FP_ERR_ON_PASS_IN_ARGUMENT;
        return result;
    }

    /*
     * Full packet size:
     *
     * Header     2
     * Address    4
     * PID        1
     * Length     2
     * Content    N
     * Checksum   2
     *
     * total = 11 + N
     */
    // check if the total packet size exceeds the maximum allowed size
    if((11u + content_len) > FP_TX_PACKET_MAX_SIZE) {
        result.status = FP_ERR_ON_PASS_IN_ARGUMENT;
        return result;
    }

    // create a buffer to hold the entire packet to be sent
    uint8_t packet[FP_TX_PACKET_MAX_SIZE];
    uint16_t checksum = 0;
    uint16_t index = 0;

    // build the header(2 bytes)
    fp_put_u16(&packet[index], FP_HEADER);
    index += 2;

    // build the address(4 bytes)
    fp_put_u32(&packet[index],FP_ADDRESS);
    index += 4;

    // build the PID(packet identifier)(1 byte)
    packet[index++] = FP_PID_COMMAND;
    checksum += (uint16_t)FP_PID_COMMAND;

    // build the length field
    // length = content_len + checksum(2 bytes)
    uint16_t length = (uint16_t)(content_len + 2);
    fp_put_u16(&packet[index], length);
    index += 2;
    checksum += (uint16_t)(length >> 8);
    checksum += (uint16_t)(length & 0xFF);

    // build the command and parameters
    for(uint16_t i = 0; i < content_len; i++)
    {
        packet[index++] = content[i];
        checksum += content[i];
    }

    // build the checksum
    fp_put_u16(&packet[index], checksum);
    index += 2;
    
    // UART send the packet
    if(usart_write_buffer_wait(&fp_usart_instance, packet, index) != STATUS_OK)
    {
        result.status = FP_ERR_UART_WRITE;
        return result;
    }

    return result;
}


static fp_result_t fp_read_ack(uint8_t *instruction_code)
{
    fp_result_t result = {0};

    // check if the instruction_code pointer is NULL
    if(instruction_code != NULL) {
        result.status = FP_ERR_ON_PASS_IN_ARGUMENT;
        return result;
    }


    // read the first 9 bytes of the acknowledgment packet
    uint8_t head[9];
    if (usart_read_buffer_wait(&fp_usart_instance, head, 9) != STATUS_OK) {
        result.status = FP_ERR_UART_READ;
        return result;
    }

    // get the Length(2 bytes) from the acknowledgment packet
    uint16_t length = ((uint16_t)head[7] << 8) | head[8];

    // read the remaining bytes of the acknowledgment packet
    uint8_t remaining[length];
    if (usart_read_buffer_wait(&fp_usart_instance, remaining, length) != STATUS_OK) {
        result.status = FP_ERR_UART_READ;
        return result;
    }

    // Check if the PID is correct for acknowledgment packets
    if(head[6] != FP_PID_ACK) {
        result.status = FP_ERR_WRONG_PID;
        return result;
    }

    // extract the instruction code from the acknowledgment packet
    result.confirmation_code = remaining[0];

    if(instruction_code == FP_CMD_SEARCH) {
        result.page_id = ((uint16_t)remaining[1] << 8) | remaining[2];
    }

    return result;
}