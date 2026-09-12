/**************************************************************************//**
 * @file      serial_console.h
 * @brief     Serial console implementation
 * @author    Tony Yan 
 * @date      2026-09-10
 ******************************************************************************/

#ifndef SERIAL_CONSOLE_H
#define SERIAL_CONSOLE_H

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************
 * Includes
 ******************************************************************************/
#include <asf.h>
#include <string.h>
#include <stdarg.h>

/******************************************************************************
 * Defines
 ******************************************************************************/

/******************************************************************************
 * Structures and Enumerations
 ******************************************************************************/
typedef enum {
    LOG_INFO_LVL = 0,   // Logs an INFO message
    LOG_DEBUG_LVL,      // Logs a DEBUG message  
    LOG_WARNING_LVL,    // Logs a WARNING message
    LOG_ERROR_LVL,      // Logs an ERROR message
    LOG_FATAL_LVL,      // Logs a FATAL message (non-recoverable error)
    LOG_OFF_LVL,        // Disables logging
    N_DEBUG_LEVELS      // Maximum number of log levels
} log_level_t;

/******************************************************************************
 * Global Function Declaration
 ******************************************************************************/
/**
 * @brief Initializes the serial console for UART communication. This function sets up the USART hardware, configures the necessary callbacks, and initializes the ring buffers for both receiving and transmitting data. It also creates a semaphore for RX operations.
 *
 * @param[in] None
 * @param[out] None
 *
 * @return None
 */
void serial_console_init(void);

/**
 * @brief Deinitializes the serial console, disabling the USART hardware and cleaning up any resources used for UART communication.
 *
 * @param[in] None
 * @param[out] None
 *
 * @return None
 */
void serial_console_deinit(void);

// Writes a string to TX buffer and sends to UART
void SerialConsoleWriteString(const char *string);

// Reads one character from RX buffer (non-blocking). Returns -1 if none.
int SerialConsoleReadCharacter(uint8_t *rxChar);

// Logs a formatted string at the given log level
void LogMessage(log_level_t level, const char *format, ...);

// Shortcut debug log (assumed DEBUG level)
void LogMessageDebug(const char *format, ...);

// Sets the minimum log level to be printed
void setLogLevel(log_level_t debugLevel);

// Gets current log level
log_level_t getLogLevel(void);

// Returns pointer to usart_module (if needed externally)
struct usart_module *GetUsartModule(void);

// Retrieves the RX semaphore used by CLI (if applicable)
SemaphoreHandle_t GetSerialRxSemaphore(void);


#ifdef __cplusplus
}
#endif

#endif /* SERIAL_CONSOLE_H */
