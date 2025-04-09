/**
 * @file      CliThread.c
 * @brief     File for the CLI Thread handler. Uses FREERTOS + CLI
 * @date      2020-02-15
 */

#include "CliThread.h"
#include "I2cDriver/I2cDriver.h"
#include "WifiHandlerThread/WifiHandler.h"
#include "SerialConsole.h"

/******************************************************************************
 * Defines
 ******************************************************************************/
#define FIRMWARE_VERSION "0.0.1"

/******************************************************************************
 * Static Command Buffers
 ******************************************************************************/
static const char pcWelcomeMessage[] =
    "FreeRTOS CLI.\r\nType Help to view a list of registered commands.\r\n";

static char bufCli[CLI_MSG_LEN];

/******************************************************************************
 * CLI Command Definitions
 ******************************************************************************/
const CLI_Command_Definition_t xClearScreen = {
    CLI_COMMAND_CLEAR_SCREEN,
    CLI_HELP_CLEAR_SCREEN,
    CLI_CALLBACK_CLEAR_SCREEN,
    CLI_PARAMS_CLEAR_SCREEN};

static const CLI_Command_Definition_t xResetCommand = {
    "reset",
    "reset: Resets the device\r\n",
    (const pdCOMMAND_LINE_CALLBACK)CLI_ResetDevice,
    0};

static const CLI_Command_Definition_t xOTAUCommand = {
    "fw",
    "fw: Download a file and perform an FW update\r\n",
    (const pdCOMMAND_LINE_CALLBACK)CLI_OTAU,
    0};

static const CLI_Command_Definition_t xI2cScanCommand = {
    "i2c",
    "i2c: Scan I2C bus for devices\r\n",
    (const pdCOMMAND_LINE_CALLBACK)CLI_i2cScan,
    0};

static const CLI_Command_Definition_t xVersionCommand = {
    "version",
    "version: Prints the firmware version.\r\n",
    (const pdCOMMAND_LINE_CALLBACK)CLI_Version,
    0};

static const CLI_Command_Definition_t xTicksCommand = {
    "ticks",
    TICK_CMD_HELP,
    (const pdCOMMAND_LINE_CALLBACK)CLI_Ticks,
    0};
	
/******************************************************************************
 * Blocking Read Function for CLI
 ******************************************************************************/
static void FreeRTOS_read(char *character)
{
    SemaphoreHandle_t sem = GetSerialRxSemaphore();
    if (character == NULL) return;

    if (xSemaphoreTake(sem, portMAX_DELAY) == pdTRUE)
    {
        if (SerialConsoleReadCharacter((uint8_t *)character) == -1)
        {
            *character = '\0';
        }
    }
}


/******************************************************************************
 * FreeRTOS CLI Core Task
 ******************************************************************************/
void vCommandConsoleTask(void *pvParameters)
{
    FreeRTOS_CLIRegisterCommand(&xClearScreen);
    FreeRTOS_CLIRegisterCommand(&xResetCommand);
    FreeRTOS_CLIRegisterCommand(&xOTAUCommand);
    FreeRTOS_CLIRegisterCommand(&xI2cScanCommand);
    FreeRTOS_CLIRegisterCommand(&xVersionCommand);
    FreeRTOS_CLIRegisterCommand(&xTicksCommand);

    static char pcOutputString[MAX_OUTPUT_LENGTH_CLI];
    static char pcInputString[MAX_INPUT_LENGTH_CLI];
    static char pcLastCommand[MAX_INPUT_LENGTH_CLI];
    static char pcEscapeCodes[CLI_PC_ESCAPE_CODE_SIZE];
    static bool isEscapeCode = false;
    static uint8_t pcEscapeCodePos = 0;

    unsigned char cInputIndex = 0;
    BaseType_t xMoreDataToFollow;
    char cRxedChar[2];

    SerialConsoleWriteString((char *)pcWelcomeMessage);

    for (;;)
    {
        FreeRTOS_read(&cRxedChar[0]);

        if (cRxedChar[0] == '\n' || cRxedChar[0] == '\r')
        {
            SerialConsoleWriteString("\r\n");
            isEscapeCode = false;
            pcEscapeCodePos = 0;

            strncpy(pcLastCommand, pcInputString, MAX_INPUT_LENGTH_CLI - 1);
            pcLastCommand[MAX_INPUT_LENGTH_CLI - 1] = 0;

            do
            {
                xMoreDataToFollow = FreeRTOS_CLIProcessCommand(
                    pcInputString, pcOutputString, MAX_OUTPUT_LENGTH_CLI);

                pcOutputString[MAX_OUTPUT_LENGTH_CLI - 1] = 0;
                SerialConsoleWriteString(pcOutputString);

            } while (xMoreDataToFollow != pdFALSE);

            cInputIndex = 0;
            memset(pcInputString, 0x00, MAX_INPUT_LENGTH_CLI);
            memset(pcOutputString, 0, MAX_OUTPUT_LENGTH_CLI);
        }
        else
        {
            if (isEscapeCode)
            {
                if (pcEscapeCodePos < CLI_PC_ESCAPE_CODE_SIZE)
                {
                    pcEscapeCodes[pcEscapeCodePos++] = cRxedChar[0];
                }
                else
                {
                    isEscapeCode = false;
                    pcEscapeCodePos = 0;
                }

                if (pcEscapeCodePos >= CLI_PC_MIN_ESCAPE_CODE_SIZE)
                {
                    if (strcasecmp(pcEscapeCodes, "oa") == 0)
                    {
                        sprintf(pcInputString, "%c[2K\r>", 27);
                        SerialConsoleWriteString(pcInputString);
                        cInputIndex = 0;
                        memset(pcInputString, 0x00, MAX_INPUT_LENGTH_CLI);
                        strncpy(pcInputString, pcLastCommand, MAX_INPUT_LENGTH_CLI - 1);
                        cInputIndex = strlen(pcInputString);
                        SerialConsoleWriteString(pcInputString);
                    }

                    isEscapeCode = false;
                    pcEscapeCodePos = 0;
                }
            }
            else if (cRxedChar[0] == ASCII_BACKSPACE || cRxedChar[0] == ASCII_DELETE)
            {
                char erase[4] = {0x08, 0x20, 0x08, 0x00};
                SerialConsoleWriteString(erase);
                if (cInputIndex > 0)
                {
                    cInputIndex--;
                    pcInputString[cInputIndex] = 0;
                }
            }
            else if (cRxedChar[0] == ASCII_ESC)
            {
                isEscapeCode = true;
                pcEscapeCodePos = 0;
            }
            else
            {
                if (cInputIndex < MAX_INPUT_LENGTH_CLI)
                {
                    pcInputString[cInputIndex++] = cRxedChar[0];
                }

                cRxedChar[1] = 0;
                SerialConsoleWriteString(cRxedChar);
            }
        }
    }
}


/******************************************************************************
 * CLI Command Implementations
 ******************************************************************************/
BaseType_t xCliClearTerminalScreen(char *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString)
{
    snprintf(bufCli, CLI_MSG_LEN - 1, "%c[2J", ASCII_ESC);
    snprintf(pcWriteBuffer, xWriteBufferLen, bufCli);
    return pdFALSE;
}

BaseType_t CLI_ResetDevice(int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString)
{
    vTaskDelay(pdMS_TO_TICKS(30));
    system_reset();
    return pdFALSE;
}

BaseType_t CLI_OTAU(int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString)
{
    WifiHandlerSetState(WIFI_DOWNLOAD_INIT);
    return pdFALSE;
}

BaseType_t CLI_i2cScan(int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString)
{
    I2C_Data i2cDevice;
    uint8_t dataOut[2] = {0, 0}, dataIn[2];

    SerialConsoleWriteString("0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\r\n");

    for (int i = 0; i < 128; i += 16)
    {
        snprintf(bufCli, CLI_MSG_LEN - 1, "%02x: ", i);
        SerialConsoleWriteString(bufCli);

        for (int j = 0; j < 16; j++)
        {
            i2cDevice.address = (i + j) << 1;
            i2cDevice.msgOut = dataOut;
            i2cDevice.msgIn = dataIn;
            i2cDevice.lenOut = 1;
            i2cDevice.lenIn = 1;

            int32_t ret = I2cWriteDataWait(&i2cDevice, 100);
            SerialConsoleWriteString((ret == 0) ? "O " : "X ");
        }
        SerialConsoleWriteString("\r\n");
    }

    return pdFALSE;
}

BaseType_t CLI_Version(char *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString)
{
    snprintf(pcWriteBuffer, xWriteBufferLen, "Firmware Version: %s\r\n", FIRMWARE_VERSION);
    return pdFALSE;
}

BaseType_t CLI_Ticks(char *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString)
{
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED)
    {
        snprintf(pcWriteBuffer, xWriteBufferLen, "Scheduler not started yet.\r\n");
    }
    else
    {
        TickType_t tickCount = xTaskGetTickCount();
        snprintf(pcWriteBuffer, xWriteBufferLen, "Ticks since start: %lu\r\n", (unsigned long)tickCount);
    }

    return pdFALSE;
}
