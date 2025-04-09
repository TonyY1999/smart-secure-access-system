/**************************************************************************
 * @file        SerialConsole.h
 * @ingroup     Serial Console
 * @brief       Serial CLI & Debugger interface via UART (115200 8N1).
 * @details     Initializes SERCOM UART + CLI + Debug logger + RX/TX ringbuffers.
 *              Used to send/receive debug logs and user commands via terminal.
 *              Includes:
 *              -- SERCOM UART @115200
 *              -- CLI command interpreter
 *              -- Debug logger (log level controlled)
 *              -- TX/RX buffers + interrupt callbacks
 *
 * @author
 * @date        January 26, 2019
 * @version     0.2 (merged)
 **************************************************************************/

#ifndef SERIAL_CONSOLE_H
#define SERIAL_CONSOLE_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/* Includes */
/******************************************************************************/
#include <asf.h>
#include <string.h>
#include <stdarg.h>
#include "circular_buffer.h"

/******************************************************************************/
/* Enumerations */
/******************************************************************************/
enum eDebugLogLevels {
    LOG_INFO_LVL    = 0, /**< Logs an INFO message */
    LOG_DEBUG_LVL   = 1, /**< Logs a DEBUG message */
    LOG_WARNING_LVL = 2, /**< Logs a WARNING message */
    LOG_ERROR_LVL   = 3, /**< Logs an ERROR message */
    LOG_FATAL_LVL   = 4, /**< Logs a FATAL message (non-recoverable error) */
    LOG_OFF_LVL     = 5, /**< Disables logging */
    N_DEBUG_LEVELS  = 6  /**< Maximum number of log levels */
};

/******************************************************************************/
/* Global Function Declarations */
/******************************************************************************/

/// Initializes UART + CLI logger
void InitializeSerialConsole(void);

/// Deinitializes UART
void DeinitializeSerialConsole(void);

/// Writes a string to TX buffer and sends to UART
void SerialConsoleWriteString(const char *string);

/// Reads one character from RX buffer (non-blocking). Returns -1 if none.
int SerialConsoleReadCharacter(uint8_t *rxChar);

/// Logs a formatted string at the given log level
void LogMessage(enum eDebugLogLevels level, const char *format, ...);

/// Shortcut debug log (assumed DEBUG level)
void LogMessageDebug(const char *format, ...);

/// Sets the minimum log level to be printed
void setLogLevel(enum eDebugLogLevels debugLevel);

/// Gets current log level
enum eDebugLogLevels getLogLevel(void);

/// Returns pointer to usart_module (if needed externally)
struct usart_module *GetUsartModule(void);

/// Retrieves the RX semaphore used by CLI (if applicable)
SemaphoreHandle_t GetSerialRxSemaphore(void);

/******************************************************************************/
/* End */
/******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* SERIAL_CONSOLE_H */
