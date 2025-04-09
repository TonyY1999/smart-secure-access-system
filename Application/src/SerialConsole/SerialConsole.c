#include "SerialConsole.h"
#include "CliThread/CliThread.h"

/******************************************************************************
 * Defines
 ******************************************************************************/
#define RX_BUFFER_SIZE 512
#define TX_BUFFER_SIZE 512

/******************************************************************************
 * Global Variables
 ******************************************************************************/
struct usart_module usart_instance;
char rxCharacterBuffer[RX_BUFFER_SIZE];
char txCharacterBuffer[TX_BUFFER_SIZE];
cbuf_handle_t cbufRx;
cbuf_handle_t cbufTx;
char latestRx;
char latestTx;
enum eDebugLogLevels currentDebugLevel = LOG_INFO_LVL;

static SemaphoreHandle_t xRxSemaphore = NULL;

/******************************************************************************
 * Callback Declarations
 ******************************************************************************/
void usart_write_callback(struct usart_module *const usart_module);
void usart_read_callback(struct usart_module *const usart_module);

/******************************************************************************
 * Local Function Declarations
 ******************************************************************************/
static void configure_usart(void);
static void configure_usart_callbacks(void);

/******************************************************************************
 * Global Functions
 ******************************************************************************/
void InitializeSerialConsole(void) {
    cbufRx = circular_buf_init((uint8_t *)rxCharacterBuffer, RX_BUFFER_SIZE);
    cbufTx = circular_buf_init((uint8_t *)txCharacterBuffer, TX_BUFFER_SIZE);

    configure_usart();
    configure_usart_callbacks();
    NVIC_SetPriority(SERCOM4_IRQn, 10);  // 可根据实际 SERCOM 调整

    usart_read_buffer_job(&usart_instance, (uint8_t *)&latestRx, 1);

    xRxSemaphore = xSemaphoreCreateBinary();
    if (xRxSemaphore == NULL) {
        SerialConsoleWriteString("ERROR: Could not create RX semaphore!\r\n");
    }
}

void DeinitializeSerialConsole(void) {
    usart_disable(&usart_instance);
}

void SerialConsoleWriteString(const char *string) {
    vTaskSuspendAll();
    if (string != NULL) {
        for (size_t i = 0; i < strlen(string); i++) {
            circular_buf_put(cbufTx, string[i]);
        }

        if (usart_get_job_status(&usart_instance, USART_TRANSCEIVER_TX) == STATUS_OK) {
            if (circular_buf_get(cbufTx, (uint8_t *)&latestTx) == 0) {
                usart_write_buffer_job(&usart_instance, (uint8_t *)&latestTx, 1);
            }
        }
    }
    xTaskResumeAll();
}

int SerialConsoleReadCharacter(uint8_t *rxChar) {
    vTaskSuspendAll();
    int result = circular_buf_get(cbufRx, rxChar);
    xTaskResumeAll();
    return result;
}

enum eDebugLogLevels getLogLevel(void) {
    return currentDebugLevel;
}

void setLogLevel(enum eDebugLogLevels debugLevel) {
    currentDebugLevel = debugLevel;
}

void LogMessage(enum eDebugLogLevels level, const char *format, ...) {
    if (level < currentDebugLevel || level == LOG_OFF_LVL) {
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
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    SerialConsoleWriteString(buffer);
    SerialConsoleWriteString("\r\n");
}

void LogMessageDebug(const char *format, ...) {
    va_list args;
    va_start(args, format);
    LogMessage(LOG_DEBUG_LVL, format, args);
    va_end(args);
}

SemaphoreHandle_t GetSerialRxSemaphore(void) {
    return xRxSemaphore;
}

struct usart_module *GetUsartModule(void) {
    return &usart_instance;
}

/******************************************************************************
 * Local Functions
 ******************************************************************************/
static void configure_usart(void) {
    struct usart_config config_usart;
    usart_get_config_defaults(&config_usart);

    config_usart.baudrate = 115200;
    config_usart.mux_setting = EDBG_CDC_SERCOM_MUX_SETTING;
    config_usart.pinmux_pad0 = EDBG_CDC_SERCOM_PINMUX_PAD0;
    config_usart.pinmux_pad1 = EDBG_CDC_SERCOM_PINMUX_PAD1;
    config_usart.pinmux_pad2 = EDBG_CDC_SERCOM_PINMUX_PAD2;
    config_usart.pinmux_pad3 = EDBG_CDC_SERCOM_PINMUX_PAD3;

    while (usart_init(&usart_instance, EDBG_CDC_MODULE, &config_usart) != STATUS_OK);

    usart_enable(&usart_instance);
}

static void configure_usart_callbacks(void) {
    usart_register_callback(&usart_instance, usart_write_callback, USART_CALLBACK_BUFFER_TRANSMITTED);
    usart_register_callback(&usart_instance, usart_read_callback, USART_CALLBACK_BUFFER_RECEIVED);
    usart_enable_callback(&usart_instance, USART_CALLBACK_BUFFER_TRANSMITTED);
    usart_enable_callback(&usart_instance, USART_CALLBACK_BUFFER_RECEIVED);
}

/******************************************************************************
 * Callbacks
 ******************************************************************************/
void usart_read_callback(struct usart_module *const usart_module) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (circular_buf_put2(cbufRx, latestRx) == 0) {
        xSemaphoreGiveFromISR(xRxSemaphore, &xHigherPriorityTaskWoken);
    }

    while (usart_read_buffer_job(&usart_instance, (uint8_t *)&latestRx, 1) != STATUS_OK);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void usart_write_callback(struct usart_module *const usart_module) {
    if (circular_buf_get(cbufTx, (uint8_t *)&latestTx) != -1) {
        usart_write_buffer_job(&usart_instance, (uint8_t *)&latestTx, 1);
    }
}
