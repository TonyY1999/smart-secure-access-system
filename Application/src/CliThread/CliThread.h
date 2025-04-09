/******************************************************************************/ /**
 * @file      CliThread.h
 * @brief     Header for the CLI Thread handler using FreeRTOS + CLI
 * @author    Eduardo Garcia
 * @date      2020-02-15
 ******************************************************************************/

#pragma once

// clang-format off
#include "asf.h"
#include "FreeRTOS_CLI.h"
#include "SerialConsole.h"
// clang-format on

/******************************************************************************
 * Task Config
 ******************************************************************************/
#define CLI_TASK_SIZE     400                          ///< Stack size for CLI task
#define CLI_PRIORITY      (configMAX_PRIORITIES - 1)   ///< CLI thread runs at highest priority
#define CLI_TASK_DELAY    150                          ///< Delay between polling (ms)

/******************************************************************************
 * CLI Buffers
 ******************************************************************************/
#define MAX_INPUT_LENGTH_CLI    100
#define MAX_OUTPUT_LENGTH_CLI   130

/******************************************************************************
 * PC Keyboard Control Sequences
 ******************************************************************************/
#define CLI_MSG_LEN                  16
#define CLI_PC_ESCAPE_CODE_SIZE     4
#define CLI_PC_MIN_ESCAPE_CODE_SIZE 2

/******************************************************************************
 * ASCII Control Codes
 ******************************************************************************/
#define ASCII_BACKSPACE     0x08
#define ASCII_DELETE        0x7F
#define ASCII_WHITESPACE    0x20
#define ASCII_ESC           27

/******************************************************************************
 * CLI Command Declarations
 ******************************************************************************/
void vCommandConsoleTask(void *pvParameters);
void CliCharReadySemaphoreGiveFromISR(void);

// Core CLI Commands
BaseType_t xCliClearTerminalScreen(char *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString);
BaseType_t CLI_ResetDevice(int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString);
BaseType_t CLI_OTAU(int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString);
BaseType_t CLI_GetImuData(int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString);
BaseType_t CLI_NeotrellisSetLed(int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString);
BaseType_t CLI_NeotrellProcessButtonBuffer(int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString);
BaseType_t CLI_DistanceSensorGetDistance(int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString);
BaseType_t CLI_SendDummyGameData(int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString);
BaseType_t CLI_Version(char *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString);
BaseType_t CLI_Ticks(char *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString);

BaseType_t CLI_i2cScan(int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString);


/******************************************************************************
 * CLI Command Registration
 ******************************************************************************/
#define CLI_COMMAND_CLEAR_SCREEN      "cls"
#define CLI_HELP_CLEAR_SCREEN         "cls: Clears the terminal screen\r\n"
#define CLI_CALLBACK_CLEAR_SCREEN     (pdCOMMAND_LINE_CALLBACK)xCliClearTerminalScreen
#define CLI_PARAMS_CLEAR_SCREEN       0
#define TICK_CMD_HELP                 "ticks: Prints the number of ticks since the scheduler started.\r\n"
