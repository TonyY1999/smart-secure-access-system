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

#include "FreeRTOS.h"
#include "I2cDriver\I2cDriver.h"
#include "SerialConsole.h"
#include "asf.h"
#include "driver/include/m2m_wifi.h"
#include "main.h"
#include "stdio_serial.h"
#include "SerialConsole/SerialConsole.h"
#include "MCHP_ATWx.h"

#include "CliThread/CliThread.h"
#include "WifiHandlerThread/WifiHandler.h"
#include "servo_driver/servo_driver.h"
#include "fingerprint_driver/fingerprint_driver.h"
#include "imu_driver/adxl345_imu.h"
#include "LCD/ST7735.h"
#include "LCD/LCD_GFX.h"

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
TaskHandle_t servoTaskHandle = NULL;		//!< Serco task handle


char bufferPrint[64];   ///< Buffer for daemon task

/******************************************************************************
 * Forward Declarations
 ******************************************************************************/
void vApplicationIdleHook(void);
void vApplicationTickHook(void);
void vApplicationStackOverflowHook(void);
void vApplicationMallocFailedHook(void);
void vApplicationDaemonTaskStartupHook(void);

static void StartTasks(void);  //!< Initial task used to initialize HW before other tasks are initialized

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

/******************************************************************************
 * Local Functions
 ******************************************************************************/
/**
 * function          StartTasks
 * @brief            Initialize application tasks
 */
static void StartTasks(void) {
    snprintf(bufferPrint, 64, "Heap before starting tasks: %d\r\n", xPortGetFreeHeapSize());
    SerialConsoleWriteString(bufferPrint);

    // initialize CLI task here
    if (xTaskCreate(vCommandConsoleTask, "CLI_TASK", CLI_TASK_SIZE, NULL, 0, &cliTaskHandle) != pdPASS) {
	    SerialConsoleWriteString("ERR: CLI task could not be initialized!\r\n");
    }
    snprintf(bufferPrint, 64, "Heap after starting CLI: %d\r\n", xPortGetFreeHeapSize());
    SerialConsoleWriteString(bufferPrint);
	
	// initialize WIFI task here
    //if (xTaskCreate(vWifiTask, "WIFI_TASK", WIFI_TASK_SIZE, NULL, 3, &wifiTaskHandle) != pdPASS) {
	    //SerialConsoleWriteString("ERR: WIFI task could not be initialized!\r\n");
    //}
    //snprintf(bufferPrint, 64, "Heap after starting WIFI: %d\r\n", xPortGetFreeHeapSize());
    //SerialConsoleWriteString(bufferPrint);
	
	// initialize IMU task here
	//if (xTaskCreate(vIMUTask, "IMU_TASK", 512, NULL, 1, NULL) != pdPASS) {
		//SerialConsoleWriteString("ERR: IMU task could not be initialized!\r\n");
	//}
	//snprintf(bufferPrint, 64, "Heap after starting IMU: %d\r\n", xPortGetFreeHeapSize());
	//SerialConsoleWriteString(bufferPrint);
	
	// initialize servo motor task here
	if (xTaskCreate(servo_task, "SERVO_TASK", 256, NULL, 1, &servoTaskHandle) != pdPASS) {
		SerialConsoleWriteString("ERR: Servo task could not be initialized!\r\n");
	}
	snprintf(bufferPrint, 64, "Heap after starting servo motor: %d\r\n", xPortGetFreeHeapSize());
	SerialConsoleWriteString(bufferPrint);
	
	// initialize fingerprint module task here
	if (xTaskCreate(fingerprint_task, "FINGERPRINT_TASK", 512, NULL, 2, NULL) != pdPASS) {
		SerialConsoleWriteString("ERR: Fingerprint task could not be initialized!\r\n");
	}
	snprintf(bufferPrint, 64, "Heap after starting fingerprint module: %d\r\n", xPortGetFreeHeapSize());
	SerialConsoleWriteString(bufferPrint);
}

/******************************************************************************
 * Callback Functions
 ******************************************************************************/
/**
 * function         vApplicationDaemonTaskStartupHook
 * @brief           Initialization code for all subsystems that require FreeRToS
 * @details			This function is called from the FreeRToS timer task. Any code
 *					here will be called before other tasks are initialized.
 */
void vApplicationDaemonTaskStartupHook(void) {
	// initialize the UART console
	InitializeSerialConsole();
	
    SerialConsoleWriteString("\x0C\n\r-----Smart Secure Access System-----\r\n");
	
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