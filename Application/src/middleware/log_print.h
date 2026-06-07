/**************************************************************************//**
 * @file      log_print.h
 * @brief     Header file for a logging system in the smart secure access system project.
 * @author    Tony Yan 
 * @date      2026-05-31
 ******************************************************************************/

#ifndef LOG_PRINT_H
#define LOG_PRINT_H 

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdint.h>
#include <stdbool.h>

/******************************************************************************
 * Defines
 ******************************************************************************/

/******************************************************************************
 * Structures and Enumerations
 ******************************************************************************/

/******************************************************************************
 * Global Function Declaration
 ******************************************************************************/
/**
 * @brief Initializes the logging system. This function must be called before any other log functions are used. It sets up the underlying debug print system and prints a header to indicate that logging has started.
 *
 * @param[in] None
 * @param[out] None
 *
 * @returns true if initialization was successful, false otherwise.
 */
bool log_init(void);

/**
 * @brief Prints a message to the log.
 *
 * @param[in] msg The message to print.
 * @param[out] None
 *
 * @returns None
 */
void log_print(const char *msg);

/**
 * @brief Prints an error message to the log, prefixed with "[ERROR]: ".
 *
 * @param[in] msg The error message to print.
 * @param[out] None
 *
 * @returns None
 */
void log_error(const char *msg);

/**
 * @brief Prints a warning message to the log, prefixed with "[WARN]: ".
 *
 * @param[in] msg The warning message to print.
 * @param[out] None
 *
 * @returns None
 */
void log_warn(const char *msg);

/**
 * @brief Prints an info message to the log, prefixed with "[INFO]: ".
 *
 * @param[in] msg The info message to print.
 * @param[out] None
 *
 * @returns None
 */
void log_info(const char *msg);

/**
 * @brief Prints a debug message to the log, prefixed with "[DEBUG]: ".
 *
 * @param[in] msg The debug message to print.
 * @param[out] None
 *
 * @returns None
 */
void log_debug(const char *msg);

#ifdef __cplusplus
}
#endif

#endif  //LOG_PRINT_H