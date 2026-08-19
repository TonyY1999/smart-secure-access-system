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

/******************************************************************************
 * Variables
 ******************************************************************************/
static bool log_initialized = false;

/******************************************************************************
 * Forward Declarations
 ******************************************************************************/

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

void log_print(const char *msg)
{
    if(!log_initialized || msg == NULL) {
        return;
    }

    dbg_print_str(msg);
}

void log_error(const char *msg) 
{
    if(!log_initialized || msg == NULL) {
        return;
    }

    dbg_print_str("[ERROR]: ");
    dbg_print_str(msg);
    dbg_print_str("\r\n");
}

void log_warn(const char *msg)
{
    if(!log_initialized || msg == NULL) {
        return;
    }

    dbg_print_str("[WARN]: ");
    dbg_print_str(msg);
    dbg_print_str("\r\n");
}

void log_info(const char *msg)
{
    if(!log_initialized || msg == NULL) {
        return;
    }

    dbg_print_str("[INFO]: ");
    dbg_print_str(msg);
    dbg_print_str("\r\n");
}

void log_debug(const char *msg)
{
    if(!log_initialized || msg == NULL) {
        return;
    }

    dbg_print_str("[DEBUG]: ");
    dbg_print_str(msg);
    dbg_print_str("\r\n");
}

/******************************************************************************
 * Local Functions
 ******************************************************************************/

/******************************************************************************
 * Callback Functions
 ******************************************************************************/
