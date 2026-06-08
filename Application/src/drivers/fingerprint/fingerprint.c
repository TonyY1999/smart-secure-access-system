/**************************************************************************//**
 * @file      fingerprint_driver.c
 * @brief     Source file for fingerprint driver functions in the Smart Secure Access System.
 * @author    Tony Yan & Yue Zhang
 * @date      2025-04-16
 ******************************************************************************/

/******************************************************************************
 * Includes
 ******************************************************************************/
#include "fingerprint_driver.h"
#include "asf.h"

/******************************************************************************
 * Defines
 ******************************************************************************/
#define FP_HEADER_HIGH	0xEF
#define FP_HEADER_LOW	0x01
#define FP_ADDRESS		0xFFFF

#define FP_PID_COMMAND         0x01
#define FP_PID_ACK             0x07

#define FP_CMD_GEN_IMAGE       0x01
#define FP_CMD_IMG2TZ          0x02
#define FP_CMD_SEARCH          0x04
#define FP_CMD_REG_MODEL       0x05
#define FP_CMD_STORE           0x06
#define FP_CMD_LOAD_CHAR       0x07
#define FP_CMD_DELETE          0x0C
#define FP_CMD_EMPTY           0x0D
#define FP_CMD_TEMPLATE_NUM    0x1D

#define FP_RX_TIMEOUT_COUNT    200000
#define FP_ENROLL_WAIT_COUNT   50000

/******************************************************************************
 * Variables
 ******************************************************************************/
static struct usart_module fingerprint_usart;
static SemaphoreHandle_t rx_semaphore;

extern TaskHandle_t servoTaskHandle;

uint8_t add_id = 0;
uint8_t delete_id = 0;

/******************************************************************************
 * Forward Declarations
 ******************************************************************************/
// Helper functions to convert between byte arrays and integers
static void fp_put_u16(uint8_t *buf, uint16_t value);

// Helper functions to convert between byte arrays and integers
static void fp_put_u32(uint8_t *buf, uint32_t value);

// Helper function to convert a byte array to a 16-bit integer
static uint16_t fp_get_u16(const uint8_t *buf);

// static void fingerprint_send_packet(uint8_t *packet, uint16_t length);
// static int fingerprint_read_response(uint8_t *buffer, uint16_t length);

// static void fingerprint_read_callback(struct usart_module *const usart_module);

/******************************************************************************
 * Global Functions
 ******************************************************************************/
bool fingerprint_init()
{	
	struct usart_config config_usart;
	usart_get_config_defaults(&config_usart);

	config_usart.baudrate = 9600;
	config_usart.mux_setting = USART_RX_1_TX_0_XCK_1;
	config_usart.pinmux_pad0 = PINMUX_PA08C_SERCOM0_PAD0;  // TX
	config_usart.pinmux_pad1 = PINMUX_PA09C_SERCOM0_PAD1;  // RX
	config_usart.pinmux_pad2 = PINMUX_UNUSED;
	config_usart.pinmux_pad3 = PINMUX_UNUSED;

	if(usart_init(&fingerprint_usart, SERCOM0, &config_usart) != STATUS_OK) {
		return false;
	}

	usart_enable(&fingerprint_usart);

	deley_ms(250); // Wait for the sensor to initialize

	return true;
}


fp_status_t gen_img() {
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
	while(fingerprint_read_response(ack, sizeof(ack)) != STATUS_OK) {
		vTaskDelay(pdMS_TO_TICKS(1000));
	}	
	
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

static uint16_t fp_calc_checksum(const uint8_t *packet) 
{
    uint16_t length = fp_get_u16(&packet[7]);
    uint16_t sum = 0;

    for(uint16_t i = 6; i < 9 + length - 2; i++)
    {
        sum += packet[i];
    }

    return sum;
}

static uint16_t build_command_packet(uint8_t command, const uint8_t *payload, uint16_t payload_len, uint8_t *tx) 
{
    uint16_t idx = 0;
    uint16_t packet_len;
    uint16_t checksum;

    tx[idx++] = FP_HEADER_HIGH;
    tx[idx++] = FP_HEADER_LOW;

    fp_put_u32(&tx[idx], g_fp_address);
    idx += 4;

    tx[idx++] = FP_PID_COMMAND;

    packet_len = payload_len + 3u;

    fp_put_u16(&tx[idx], packet_len);
    idx += 2;

    tx[idx++] = command;

    if(payload != NULL && payload_len > 0u)
    {
        memcpy(&tx[idx], payload, payload_len);
        idx += payload_len;
    }

    checksum = fp_calc_checksum(tx);

    fp_put_u16(&tx[idx], checksum);
    idx += 2;

    return idx;
}



// static void fingerprint_send_packet(uint8_t *packet, uint16_t length)
// {
// 	int res = usart_write_buffer_wait(&fingerprint_usart_instance, packet, length);
// }

// static int fingerprint_read_response(uint8_t* buffer, uint16_t length)
// {
// 	return usart_read_buffer_wait(&fingerprint_usart_instance, buffer, length);	
// }

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
extern volatile bool mqtt_connected;
extern TaskHandle_t wifiTaskHandle;
extern TaskHandle_t IMUTaskHandle;









static fp_status_t fp_uart_read_byte_timeout(uint8_t *data)
{
    uint32_t timeout = FP_RX_TIMEOUT_COUNT;

    while(timeout--)
    {
        if(usart_read_wait(&fingerprint_usart, data) == STATUS_OK)
        {
            return FP_OK;
        }
    }

    return FP_TIMEOUT;
}

static fp_status_t fp_uart_read_buffer_timeout(uint8_t *buf, uint16_t len)
{
    for(uint16_t i = 0; i < len; i++)
    {
        fp_status_t status = fp_uart_read_byte_timeout(&buf[i]);

        if(status != FP_OK)
        {
            return status;
        }
    }

    return FP_OK;
}

static fp_status_t fp_send_command(uint8_t command,
                                   const uint8_t *payload,
                                   uint16_t payload_len)
{
    uint8_t tx[32];
    uint16_t tx_len;

    if(payload_len > 20u)
    {
        return FP_PARAM_ERROR;
    }

    tx_len = fp_build_command_packet(command, payload, payload_len, tx);

    if(usart_write_buffer_wait(&fingerprint_usart, tx, tx_len) != STATUS_OK)
    {
        return FP_TIMEOUT;
    }

    return FP_OK;
}

static fp_status_t fp_read_packet(uint8_t expected_pid,
                                  uint8_t *rx,
                                  uint16_t rx_max_len,
                                  uint16_t *packet_len_out)
{
    fp_status_t status;
    uint16_t length;
    uint16_t total_len;
    uint16_t checksum_recv;
    uint16_t checksum_calc;

    status = fp_uart_read_buffer_timeout(rx, 9);
    if(status != FP_OK)
    {
        return status;
    }

    if(rx[0] != FP_HEADER_HIGH || rx[1] != FP_HEADER_LOW)
    {
        return FP_BAD_PACKET;
    }

    if(rx[6] != expected_pid)
    {
        return FP_BAD_PACKET;
    }

    length = fp_get_u16(&rx[7]);
    total_len = 9u + length;

    if(total_len > rx_max_len)
    {
        return FP_BAD_PACKET;
    }

    status = fp_uart_read_buffer_timeout(&rx[9], length);
    if(status != FP_OK)
    {
        return status;
    }

    checksum_recv = fp_get_u16(&rx[total_len - 2u]);
    checksum_calc = fp_calc_checksum(rx);

    if(checksum_recv != checksum_calc)
    {
        return FP_CHECKSUM_ERROR;
    }

    if(packet_len_out != NULL)
    {
        *packet_len_out = total_len;
    }

    return FP_OK;
}

static fp_status_t fp_read_ack(uint8_t *content,
                               uint16_t content_max_len,
                               uint16_t *content_len_out)
{
    uint8_t rx[64];
    uint16_t total_len;
    uint16_t length;
    uint16_t content_len;
    fp_status_t status;

    status = fp_read_packet(FP_PID_ACK, rx, sizeof(rx), &total_len);
    if(status != FP_OK)
    {
        return status;
    }

    length = fp_get_u16(&rx[7]);
    content_len = length - 2u;

    if(content_len > content_max_len)
    {
        return FP_BAD_PACKET;
    }

    memcpy(content, &rx[9], content_len);

    if(content_len_out != NULL)
    {
        *content_len_out = content_len;
    }

    return FP_OK;
}

static fp_status_t fp_execute_simple_command(uint8_t command,
                                             const uint8_t *payload,
                                             uint16_t payload_len)
{
    uint8_t content[8];
    uint16_t content_len;
    fp_status_t status;

    status = fp_send_command(command, payload, payload_len);
    if(status != FP_OK)
    {
        return status;
    }

    status = fp_read_ack(content, sizeof(content), &content_len);
    if(status != FP_OK)
    {
        return status;
    }

    if(content_len < 1u)
    {
        return FP_BAD_PACKET;
    }

    return (fp_status_t)content[0];
}

bool fp_init(void)
{
    struct usart_config config_usart;

    usart_get_config_defaults(&config_usart);

    config_usart.baudrate    = FP_UART_BAUDRATE;
    config_usart.mux_setting = FP_USART_MUX_SETTING;
    config_usart.pinmux_pad0 = FP_USART_PINMUX_PAD0;
    config_usart.pinmux_pad1 = FP_USART_PINMUX_PAD1;
    config_usart.pinmux_pad2 = FP_USART_PINMUX_PAD2;
    config_usart.pinmux_pad3 = FP_USART_PINMUX_PAD3;

    if(usart_init(&fingerprint_usart,
                  FP_USART_SERCOM,
                  &config_usart) != STATUS_OK)
    {
        return false;
    }

    usart_enable(&fingerprint_usart);

    delay_ms(250);

    return true;
}

fp_status_t fp_get_image(void)
{
    return fp_execute_simple_command(FP_CMD_GEN_IMAGE, NULL, 0);
}

fp_status_t fp_image_to_char(uint8_t buffer_id)
{
    uint8_t payload[1];

    if(buffer_id != 1u && buffer_id != 2u)
    {
        return FP_PARAM_ERROR;
    }

    payload[0] = buffer_id;

    return fp_execute_simple_command(FP_CMD_IMG2TZ, payload, sizeof(payload));
}

fp_status_t fp_create_model(void)
{
    return fp_execute_simple_command(FP_CMD_REG_MODEL, NULL, 0);
}

fp_status_t fp_store(uint8_t buffer_id, uint16_t page_id)
{
    uint8_t payload[3];

    if(buffer_id != 1u && buffer_id != 2u)
    {
        return FP_PARAM_ERROR;
    }

    payload[0] = buffer_id;
    fp_put_u16(&payload[1], page_id);

    return fp_execute_simple_command(FP_CMD_STORE, payload, sizeof(payload));
}

fp_status_t fp_load_char(uint8_t buffer_id, uint16_t page_id)
{
    uint8_t payload[3];

    if(buffer_id != 1u && buffer_id != 2u)
    {
        return FP_PARAM_ERROR;
    }

    payload[0] = buffer_id;
    fp_put_u16(&payload[1], page_id);

    return fp_execute_simple_command(FP_CMD_LOAD_CHAR, payload, sizeof(payload));
}

fp_status_t fp_search(uint8_t buffer_id,
                      uint16_t start_page,
                      uint16_t page_count,
                      fp_search_result_t *result)
{
    uint8_t payload[5];
    uint8_t content[8];
    uint16_t content_len;
    fp_status_t status;

    if(result == NULL)
    {
        return FP_PARAM_ERROR;
    }

    if(buffer_id != 1u && buffer_id != 2u)
    {
        return FP_PARAM_ERROR;
    }

    payload[0] = buffer_id;
    fp_put_u16(&payload[1], start_page);
    fp_put_u16(&payload[3], page_count);

    status = fp_send_command(FP_CMD_SEARCH, payload, sizeof(payload));
    if(status != FP_OK)
    {
        return status;
    }

    status = fp_read_ack(content, sizeof(content), &content_len);
    if(status != FP_OK)
    {
        return status;
    }

    if(content_len < 1u)
    {
        return FP_BAD_PACKET;
    }

    if(content[0] != FP_OK)
    {
        return (fp_status_t)content[0];
    }

    if(content_len < 5u)
    {
        return FP_BAD_PACKET;
    }

    result->page_id = fp_get_u16(&content[1]);
    result->score   = fp_get_u16(&content[3]);

    return FP_OK;
}

fp_status_t fp_delete(uint16_t page_id, uint16_t count)
{
    uint8_t payload[4];

    fp_put_u16(&payload[0], page_id);
    fp_put_u16(&payload[2], count);

    return fp_execute_simple_command(FP_CMD_DELETE, payload, sizeof(payload));
}

fp_status_t fp_empty_database(void)
{
    return fp_execute_simple_command(FP_CMD_EMPTY, NULL, 0);
}

fp_status_t fp_get_template_count(uint16_t *count)
{
    uint8_t content[4];
    uint16_t content_len;
    fp_status_t status;

    if(count == NULL)
    {
        return FP_PARAM_ERROR;
    }

    status = fp_send_command(FP_CMD_TEMPLATE_NUM, NULL, 0);
    if(status != FP_OK)
    {
        return status;
    }

    status = fp_read_ack(content, sizeof(content), &content_len);
    if(status != FP_OK)
    {
        return status;
    }

    if(content_len < 1u)
    {
        return FP_BAD_PACKET;
    }

    if(content[0] != FP_OK)
    {
        return (fp_status_t)content[0];
    }

    if(content_len < 3u)
    {
        return FP_BAD_PACKET;
    }

    *count = fp_get_u16(&content[1]);

    return FP_OK;
}

fp_status_t fp_enroll(uint16_t page_id)
{
    fp_status_t status;
    uint32_t wait_count;

    wait_count = 0u;
    do
    {
        status = fp_get_image();
        wait_count++;
    } while(status == FP_NO_FINGER && wait_count < FP_ENROLL_WAIT_COUNT);

    if(status != FP_OK)
    {
        return status;
    }

    status = fp_image_to_char(1);
    if(status != FP_OK)
    {
        return status;
    }

    wait_count = 0u;
    do
    {
        status = fp_get_image();
        wait_count++;
    } while(status != FP_NO_FINGER && wait_count < FP_ENROLL_WAIT_COUNT);

    if(status != FP_NO_FINGER)
    {
        return FP_TIMEOUT;
    }

    wait_count = 0u;
    do
    {
        status = fp_get_image();
        wait_count++;
    } while(status == FP_NO_FINGER && wait_count < FP_ENROLL_WAIT_COUNT);

    if(status != FP_OK)
    {
        return status;
    }

    status = fp_image_to_char(2);
    if(status != FP_OK)
    {
        return status;
    }

    status = fp_create_model();
    if(status != FP_OK)
    {
        return status;
    }

    return fp_store(1, page_id);
}

fp_status_t fp_identify(fp_search_result_t *result)
{
    fp_status_t status;

    if(result == NULL)
    {
        return FP_PARAM_ERROR;
    }

    status = fp_get_image();
    if(status != FP_OK)
    {
        return status;
    }

    status = fp_image_to_char(1);
    if(status != FP_OK)
    {
        return status;
    }

    return fp_search(1, 0, FP_MAX_TEMPLATE_NUM, result);
}
