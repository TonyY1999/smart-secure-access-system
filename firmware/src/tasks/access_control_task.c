/**************************************************************************//**
 * @file      access_control_task.c
 * @brief     Source file for access_control_task.c, defining the implementation for the access control task in the smart secure access system.
 * @author    Eduardo Garcia
 * @date      2020-01-01
 ******************************************************************************/

/******************************************************************************
 * Includes
 ******************************************************************************/
#include "access_control_task.h"
#include "access_control.h"

/******************************************************************************
 * Defines
 ******************************************************************************/
#define FINGERPRINT_SCAN_DELAY_MS 200
#define FINGERPRINT_QUEUE_WAIT_MS 50

/******************************************************************************
 * Variables
 ******************************************************************************/

/******************************************************************************
 * Forward Declarations
 ******************************************************************************/
static void fingerprint_send_event(access_event_type_t type, int finger_id);

/******************************************************************************
 * Global Functions
 ******************************************************************************/
void fingerprint_task(void *pvParameters){
	// init fingerprint module
	fingerprint_init();
	// config_servo();
	
	while (1)
	{
		int finger_id = fingerprint_search();
		
        if (finger_id != -1)
		{	
            fingerprint_send_event(ACCESS_EVENT_SUCCESS, finger_id);

			// while (!mqtt_connected ) {
			// 	SerialConsoleWriteString("waiting mqtt\r\n");
			// 	vTaskDelay(pdMS_TO_TICKS(100));
			// }
			
			//cloud_send_finger_ID(finger_id);
			
			// unlock the door
			// config_servo();
			// pwm_set_servo_angle_unlock_door();
			// vTaskDelay(pdMS_TO_TICKS(30000));
			// pwm_set_servo_angle_lock_door();
		}
		
		vTaskDelay(pdMS_TO_TICKS(3000));
	}
}

/******************************************************************************
 * Local Functions
 ******************************************************************************/
 static void fingerprint_send_event(access_event_type_t type, int finger_id)
{
    access_event_t event;
    QueueHandle_t queue;

    event.type = type;
    event.finger_id = finger_id;

    queue = access_control_get_queue();

    if (queue != NULL)
    {
        xQueueSend(
            queue,
            &event,
            pdMS_TO_TICKS(FINGERPRINT_QUEUE_WAIT_MS)
        );
    }
}

/******************************************************************************
 * Callback Functions
 ******************************************************************************/
