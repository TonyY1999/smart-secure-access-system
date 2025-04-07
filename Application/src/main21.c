/**************************************************************************//**
 * @file      main.c
 * @brief     Main application entry point for Smart Secure Access System
 * @author    Tony Yan & Yue Zhang
 * @date      2025-04-06
 ******************************************************************************/

/******************************************************************************
 * Includes
 ******************************************************************************/
#include <errno.h>

#include "CliThread/CliThread.h"
#include "FreeRTOS.h"
#include "I2cDriver\I2cDriver.h"
#include "SerialConsole.h"
#include "WifiHandlerThread/WifiHandler.h"
#include "asf.h"
#include "driver/include/m2m_wifi.h"
#include "main.h"
#include "stdio_serial.h"
#include "SerialConsole/SerialConsole.h"
#include "imu_driver/lsm6dso_reg.h"
#include "MCHP_ATWx.h"

/******************************************************************************
 * Defines
 ******************************************************************************/
#define APP_TASK_ID 0 /**< @brief ID for the application task */
#define CLI_TASK_ID 1 /**< @brief ID for the command line interface task */

/******************************************************************************
 * Variables
 ******************************************************************************/
static TaskHandle_t cliTaskHandle = NULL;       //!< CLI task handle
static TaskHandle_t daemonTaskHandle = NULL;    //!< Daemon task handle
static TaskHandle_t wifiTaskHandle = NULL;      //!< WIFI task handle
static TaskHandle_t uiTaskHandle = NULL;        //!< UI task handle
static TaskHandle_t controlTaskHandle = NULL;   //!< Control task handle

char bufferPrint[64];   ///< Buffer for daemon task

/******************************************************************************
 * Forward Declarations
 ******************************************************************************/
void vApplicationIdleHook(void);
void vApplicationTickHook(void);
void vApplicationStackOverflowHook(void);
void vApplicationMallocFailedHook(void);
void vApplicationDaemonTaskStartupHook(void);

//!< Initial task used to initialize HW before other tasks are initialized
static void StartTasks(void);

// IMU task function 
void vIMUTask(void *pvParameters)
{
	SerialConsoleWriteString("Initializing IMU...\r\n");

	if (InitImu() != 0) {
		SerialConsoleWriteString("IMU initialization failed!\r\n");
		vTaskDelete(NULL);
	}

	SerialConsoleWriteString("IMU initialized successfully.\r\n");

	stmdev_ctx_t *ctx = GetImuStruct();

	while (1) {
		int16_t x, y, z;
		uint8_t data_raw_accel[6] = {0};

		lsm6dso_acceleration_raw_get(ctx, data_raw_accel);

		x = (int16_t)(data_raw_accel[1] << 8 | data_raw_accel[0]);
		y = (int16_t)(data_raw_accel[3] << 8 | data_raw_accel[2]);
		z = (int16_t)(data_raw_accel[5] << 8 | data_raw_accel[4]);

		char msg[128];
		snprintf(msg, sizeof(msg), "Accel X: %d, Y: %d, Z: %d\r\n", x, y, z);
		SerialConsoleWriteString(msg);

		vTaskDelay(pdMS_TO_TICKS(500));
	}
}

/**
 * @brief Main application function.
 * Application entry point.
 * @return int
 */
int main(void) {
    /* Initialize the board. */
    system_init();
	
    // Initialize trace capabilities
    vTraceEnable(TRC_START);
	
    // Start FreeRTOS scheduler
    vTaskStartScheduler();

    return 0;   // Will not get here
}

/**
 * function         vApplicationDaemonTaskStartupHook
 * @brief           Initialization code for all subsystems that require FreeRToS
 * @details			This function is called from the FreeRToS timer task. Any code
 *					here will be called before other tasks are initialized.
 */
void vApplicationDaemonTaskStartupHook(void) {
	// initialize the UART console
	InitializeSerialConsole();
	
    SerialConsoleWriteString("\r\n-----Smart Secure Access System-----\r\n");

    // initialize HW that needs FreeRTOS Initialization
    SerialConsoleWriteString("Initialize I2C driver...\r\n");
    if (I2cInitializeDriver() != STATUS_OK) {
        SerialConsoleWriteString("Error initializing I2C Driver!\r\n");
    } else {
        SerialConsoleWriteString("Initialized I2C Driver!\r\n");
    }

    StartTasks();
	
    vTaskSuspend(daemonTaskHandle);
}

/**
 * function          StartTasks
 * @brief            Initialize application tasks
 */
static void StartTasks(void) {
    snprintf(bufferPrint, 64, "Heap before starting tasks: %d\r\n", xPortGetFreeHeapSize());
    SerialConsoleWriteString(bufferPrint);

    // initialize CLI task here
    if (xTaskCreate(vCommandConsoleTask, "CLI_TASK", CLI_TASK_SIZE, NULL, CLI_PRIORITY, &cliTaskHandle) != pdPASS) {
        SerialConsoleWriteString("ERR: CLI task could not be initialized!\r\n");
    }
    snprintf(bufferPrint, 64, "Heap after starting CLI: %d\r\n", xPortGetFreeHeapSize());
    SerialConsoleWriteString(bufferPrint);
	
	// initialize WIFI task here
    //if (xTaskCreate(vWifiTask, "WIFI_TASK", WIFI_TASK_SIZE, NULL, WIFI_PRIORITY, &wifiTaskHandle) != pdPASS) {
        //SerialConsoleWriteString("ERR: WIFI task could not be initialized!\r\n");
    //}
    //snprintf(bufferPrint, 64, "Heap after starting WIFI: %d\r\n", xPortGetFreeHeapSize());
    //SerialConsoleWriteString(bufferPrint);
	
	// initialize IMU task here
	if (xTaskCreate(vIMUTask, "IMU_TASK", 512, NULL, 1, NULL) != pdPASS) {
		SerialConsoleWriteString("ERR: IMU task could not be initialized!\r\n");
	}
	snprintf(bufferPrint, 64, "Heap after starting IMU: %d\r\n", xPortGetFreeHeapSize());
	SerialConsoleWriteString(bufferPrint);
}

/**
 * function          vApplicationMallocFailedHook
 * @brief            Called when a malloc() fails in FreeRTOS. Handles memory allocation failure
 */
void vApplicationMallocFailedHook(void) {
    SerialConsoleWriteString("Error on memory allocation on FREERTOS!\r\n");
    while (1);
}

/**
 * function          vApplicationStackOverflowHook
 * @brief            Called when a stack overflow is detected in a FreeRTOS task. Handles the error
 */
void vApplicationStackOverflowHook(void) {
    SerialConsoleWriteString("Error on stack overflow on FREERTOS!\r\n");
    while (1);
}

/**
 * function          vApplicationTickHook
 * @brief            Called from each RTOS tick interrupt. Used here to run MQTT-related SysTick handler
 */
void vApplicationTickHook(void) { 
	SysTick_Handler_MQTT(); 
}
