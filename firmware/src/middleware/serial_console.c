/**************************************************************************//**
 * @file      serial_console.c
 * @brief     Serial console implementation
 * @author    Tony Yan
 * @date      2026-09-10
 ******************************************************************************/

/******************************************************************************
 * Includes
 ******************************************************************************/
#include "serial_console.h"
#include "ring_buffer.h"

/******************************************************************************
 * Defines
 ******************************************************************************/
#define RX_BUFFER_SIZE 256
#define TX_BUFFER_SIZE 256

/******************************************************************************
 * Variables
 ******************************************************************************/
// USART instance for serial console communication
static struct usart_module sc_usart_instance;

// Ring buffer structures and real buffers for RX and TX data
static ring_buffer_t rx_ring;
static ring_buffer_t tx_ring;
static char rx_storage[RX_BUFFER_SIZE];
static char tx_storage[TX_BUFFER_SIZE];

// Variables to hold the latest received and transmitted characters
static char latest_rx;
static char latest_tx;

// Current debug log level, initialized to LOG_INFO_LVL
log_level_t current_debug_level = LOG_INFO_LVL;

// Semaphore handle for RX operations
static SemaphoreHandle_t xRxSemaphore = NULL;

/******************************************************************************
 * Forward Declarations
 ******************************************************************************/
// Local function declarations for USART configuration and callback setup
static bool config_usart(void);
static void config_usart_callbacks(void);

// Callback function declarations for USART read and write operations
void usart_write_callback(struct usart_module* const usart_module);
void usart_read_callback(struct usart_module* const usart_module);

/******************************************************************************
 * Global Functions
 ******************************************************************************/
void serial_console_init(void)
{
    // initialize ring buffers for RX and TX data
    if(!ring_buffer_init(&rx_ring, (uint8_t*)rx_storage, RX_BUFFER_SIZE)) {
        return;
    }

    if(!ring_buffer_init(&tx_ring, (uint8_t*)tx_storage, TX_BUFFER_SIZE)) {
        return;
    }

    // configure USART for serial console communication
    if(!config_usart()) {
        return;
    }

    // create a binary semaphore for RX operations
    xRxSemaphore = xSemaphoreCreateBinary();
    if (xRxSemaphore == NULL) {
        usart_disable(&sc_usart_instance);
        return;
    }

    // configure USART callbacks for read and write operations
    config_usart_callbacks();

    usart_read_buffer_job(&sc_usart_instance, (uint8_t*) &latest_rx, 1);
}

void serial_console_deinit(void)
{
    usart_disable(&sc_usart_instance);
}

void SerialConsoleWriteString(const char *string) 
{
    vTaskSuspendAll();
    if (string != NULL) {
        for (size_t i = 0; i < strlen(string); i++) {
            ring_buffer_put(&tx_ring, string[i]);
        }

        if (usart_get_job_status(&sc_usart_instance, USART_TRANSCEIVER_TX) == STATUS_OK) {
            if (ring_buffer_get(&tx_ring, (uint8_t*) &latest_tx) == 0) {
                usart_write_buffer_job(&sc_usart_instance, (uint8_t*) &latest_tx, 1);
            }
        }
    }
    xTaskResumeAll();
}

int SerialConsoleReadCharacter(uint8_t *rxChar) 
{
    vTaskSuspendAll();
    int result = circular_buf_get(rx_ring, rxChar);
    xTaskResumeAll();
    return result;
}

log_level_t getLogLevel(void) 
{
    return current_debug_level;
}

void setLogLevel(log_level_t debugLevel) 
{
    current_debug_level = debugLevel;
}

void LogMessage(enum eDebugLogLevels level, const char *format, ...) 
{
    if (level < current_debug_level || level == LOG_OFF_LVL) {
        return;
    }

    switch (level) {
        case LOG_INFO_LVL:    SerialConsoleWriteString("[INFO] "); break;
        case LOG_DEBUG_LVL:   SerialConsoleWriteString("[DEBUG] "); break;
        case LOG_WARNING_LVL: SerialConsoleWriteString("[WARNING] "); break;
        case LOG_ERROR_LVL:   SerialConsoleWriteString("[ERROR] "); break;
        case LOG_FATAL_LVL:   SerialConsoleWriteString("[FATAL] "); break;
        default:              SerialConsoleWriteString("[UNKNOWN] "); break;
    }

    va_list args;
    va_start(args, format);
    char buffer[64];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    SerialConsoleWriteString(buffer);
    SerialConsoleWriteString("\r\n");
}

void LogMessageDebug(const char *format, ...) 
{
    va_list args;
    va_start(args, format);
    LogMessage(LOG_DEBUG_LVL, format, args);
    va_end(args);
}

SemaphoreHandle_t GetSerialRxSemaphore(void) 
{
    return xRxSemaphore;
}

struct usart_module *GetUsartModule(void) 
{
    return &sc_usart_instance;
}

/******************************************************************************
 * Local Functions
 ******************************************************************************/
static bool config_usart(void) 
{
    struct usart_config sc_usart_config;
    usart_get_config_defaults(&sc_usart_config);

    sc_usart_config.baudrate = 115200;
    sc_usart_config.mux_setting = EDBG_CDC_SERCOM_MUX_SETTING;
    sc_usart_config.pinmux_pad0 = EDBG_CDC_SERCOM_PINMUX_PAD0;
    sc_usart_config.pinmux_pad1 = EDBG_CDC_SERCOM_PINMUX_PAD1;
    sc_usart_config.pinmux_pad2 = EDBG_CDC_SERCOM_PINMUX_PAD2;
    sc_usart_config.pinmux_pad3 = EDBG_CDC_SERCOM_PINMUX_PAD3;

    if (usart_init(&sc_usart_instance, EDBG_CDC_MODULE, &sc_usart_config) != STATUS_OK) {
        return false;
    }

    usart_enable(&sc_usart_instance);
    return true;
}

static void config_usart_callbacks(void) 
{
    usart_register_callback(&sc_usart_instance, usart_write_callback, USART_CALLBACK_BUFFER_TRANSMITTED);
    usart_register_callback(&sc_usart_instance, usart_read_callback, USART_CALLBACK_BUFFER_RECEIVED);
    usart_enable_callback(&sc_usart_instance, USART_CALLBACK_BUFFER_TRANSMITTED);
    usart_enable_callback(&sc_usart_instance, USART_CALLBACK_BUFFER_RECEIVED);
}

/******************************************************************************
 * Callback Functions
 ******************************************************************************/
void usart_write_callback(struct usart_module* const usart_module) 
{
    if (ring_buffer_get(&tx_ring, (uint8_t*) &latest_tx)) {
        usart_write_buffer_job(&sc_usart_instance, (uint8_t*) &latest_tx, 1);
    }
}

void usart_read_callback(struct usart_module* const usart_module) 
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (ring_buffer_put(&rx_ring, latest_rx)) {
        xSemaphoreGiveFromISR(xRxSemaphore, &xHigherPriorityTaskWoken);
    }

    if(usart_read_buffer_job(usart_module, (uint8_t*) &latest_rx, 1) != STATUS_OK) {
        // Handle error if needed
        return;
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
