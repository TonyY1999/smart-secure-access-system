/**************************************************************************//**
 * @file      log_print.c
 * @brief     Source file for a logging system in the smart secure access system project.
 * @author    Tony Yan 
 * @date      2026-05-31
 ******************************************************************************/

/******************************************************************************
 * Includes
 ******************************************************************************/
#include "log_print.h"
#include "dbg_print.h"

/******************************************************************************
 * Defines
 ******************************************************************************/
#define LOG_BUFFER_SIZE    128u

/******************************************************************************
 * Variables
 ******************************************************************************/
static bool log_initialized = false;

/******************************************************************************
 * Forward Declarations
 ******************************************************************************/
static void log_vprint(const char* prefix, const char* fmt, va_list args);

/******************************************************************************
 * Global Functions
 ******************************************************************************/
bool log_init(void)
{
    enum status_code status = dbg_init();
    if(status != STATUS_OK) {
        log_initialized = false;
        return false;
    }

    log_initialized = true;
    
    dbg_print_str("\r\n");
    dbg_print_str("================================\r\n");
    dbg_print_str(" SSAS Debug Log Started\r\n");
    dbg_print_str("================================\r\n");

    return true;
}

void log_print(const char* fmt, ...)
{
    va_list args;

    va_start(args, fmt);

    log_vprint(NULL, fmt, args);

    va_end(args);
}

void log_error(const char* fmt, ...)
{
    va_list args;

    va_start(args, fmt);

    log_vprint("[ERROR]: ", fmt, args);

    va_end(args);
}

void log_warn(const char* fmt, ...)
{
    va_list args;

    va_start(args, fmt);

    log_vprint("[WARN]: ", fmt, args);

    va_end(args);
}

void log_info(const char* fmt, ...)
{
    va_list args;

    va_start(args, fmt);

    log_vprint("[INFO]: ", fmt, args);

    va_end(args);
}

void log_debug(const char* fmt, ...)
{
    va_list args;

    va_start(args, fmt);

    log_vprint("[DEBUG]: ", fmt, args);

    va_end(args);
}

/******************************************************************************
 * Local Functions
 ******************************************************************************/
static void log_vprint(const char* prefix, const char* fmt, va_list args)
{
    char buffer[LOG_BUFFER_SIZE];

    if (!log_initialized || fmt == NULL){
        return;
    }

    if (prefix != NULL){
        dbg_print_str(prefix);
    }

    vsnprintf(buffer, sizeof(buffer), fmt, args);

    dbg_print_str(buffer);
    dbg_print_str("\r\n");
}

/******************************************************************************
 * Callback Functions
 ******************************************************************************/
