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
#define FP_SERCOM              SERCOM0
#define FP_SERCOM_MUX_SETTING  USART_RX_1_TX_0_XCK_1

#define FP_PINMUX_PAD0         PINMUX_PA08C_SERCOM0_PAD0  // FP_TX
#define FP_PINMUX_PAD1         PINMUX_PA09C_SERCOM0_PAD1  // FP_RX
#define FP_PINMUX_PAD2         PINMUX_UNUSED
#define FP_PINMUX_PAD3         PINMUX_UNUSED

#define FP_HEADER   0xEF01

#define FP_ADDRESS  0xFFFFFFFF

#define FP_PID_COMMAND         0x01
#define FP_PID_ACK             0x07

// Fingerprint instruction codes
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

#define FP_TX_PACKET_MAX_SIZE      64u
#define FP_ACK_DATA_MAX_SIZE       64u

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

// Helper function to convert a byte array to a 32-bit integer
static fp_driver_status_t fp_send_command(const uint8_t *content, uint16_t content_len);

/******************************************************************************
 * Global Functions
 ******************************************************************************/
fp_driver_status_t fingerprint_init(void)
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
		return FP_DRIVER_UART_ERROR;
	}

	usart_enable(&fp_usart_instance);

	delay_ms(200); // Wait for the sensor to power up and be ready for communication
    
	return FP_DRIVER_OK;
}

fp_result_t get_img(void)
{
    uint8_t command[] = {FP_CMD_GET_IMAGE};
    return fp_send_command(command, sizeof(command));
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

static uint16_t fp_checksum(uint8_t pid, uint16_t length, const uint8_t *content)
{   
    uint16_t checksum = pid + (length >> 8) + (length & 0xFF);

    for(uint16_t i = 0; i < length - 2; i++)
    {
        checksum += content[i];
    }

    return checksum;
}

static fp_driver_status_t fp_send_command(const uint8_t *content, uint16_t content_len)
{
    uint8_t packet[FP_TX_PACKET_MAX_SIZE];

    uint16_t index = 0;

    if ((content == NULL) || (content_len == 0)) {
        return FP_DRIVER_INVALID_ARG;
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
    // Build the packet
    // Build the header
    fp_put_u16(&packet[index], FP_HEADER);
    index += 2;

    // Build the address
    fp_put_u32(&packet[index],FP_ADDRESS);
    index += 4;

    // Build the PID
    packet[index++] = FP_PID_COMMAND;

    // Build the length field
    // length = content_len + checksum(2 bytes)
    uint16_t length = (uint16_t)(content_len + 2);
    fp_put_u16(&packet[index], length);
    index += 2;

    // Build the command and parameters
    for(uint16_t i = 0; i < content_len; i++)
    {
        packet[index++] = content[i];

    }

    // Build the checksum
    uint16_t checksum = fp_checksum(FP_PID_COMMAND, length, content);
    fp_put_u16(&packet[index], checksum);
    index += 2;
    
    // UART send the packet
    if(usart_write_buffer_wait(&fp_usart_instance, packet, index) != STATUS_OK)
    {
        return FP_DRIVER_UART_ERROR;

    }

    return FP_DRIVER_OK;
}

