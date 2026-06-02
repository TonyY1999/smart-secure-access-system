/**************************************************************************//**
 * @file      template.c
 * @brief     Template for ESE516 with Doxygen-style comments
 * @author    Eduardo Garcia
 * @date      2020-01-01
 ******************************************************************************/

/******************************************************************************
 * Includes
 ******************************************************************************/
#include "access_control.h"
#include "servo_driver.h"

/******************************************************************************
 * Defines
 ******************************************************************************/

/******************************************************************************
 * Variables
 ******************************************************************************/
static SemaphoreHandle_t servo_mutex = NULL;

/******************************************************************************
 * Forward Declarations
 ******************************************************************************/

/******************************************************************************
 * Global Functions
 ******************************************************************************/
void access_control_init(void)
{
    servo_init();
    buzzer_init();
    lcd_init();

    servo_mutex = xSemaphoreCreateMutex();
    configASSERT(servo_mutex != NULL);

    servo_lock();
    lcd_show_string("System Locked");
}

static void access_control_unlock_door(const access_event_t *event)
{
    if (servo_mutex == NULL)
    {
        return;
    }

    if (xSemaphoreTake(servo_mutex, pdMS_TO_TICKS(SERVO_MUTEX_WAIT_MS)) == pdTRUE)
    {
        servo_unlock();

        if (event->source == ACCESS_SOURCE_FINGERPRINT)
        {
            lcd_show_string("Fingerprint OK");
        }
        else if (event->source == ACCESS_SOURCE_REMOTE)
        {
            lcd_show_string("Remote Unlock");
        }
        else
        {
            lcd_show_string("Door Unlocked");
        }

        buzzer_beep_success();

        vTaskDelay(pdMS_TO_TICKS(DOOR_UNLOCK_TIME_MS));

        servo_lock();
        lcd_show_string("System Locked");

        xSemaphoreGive(servo_mutex);
    }
}

static void access_control_deny_access(const access_event_t *event)
{
    (void)event;

    lcd_show_string("Access Denied");
    buzzer_beep_error();
}

static void access_control_tamper_alert(const access_event_t *event)
{
    (void)event;

    lcd_show_string("Tamper Alert!");
    buzzer_alarm_on();
}

static void access_control_system_error(const access_event_t *event)
{
    (void)event;

    lcd_show_string("System Error");
    buzzer_beep_error();
}

void access_control_handle_event(const access_event_t *event)
{
    if (event == NULL)
    {
        return;
    }

    switch (event->type)
    {
        case ACCESS_EVENT_UNLOCK_REQUEST:
            access_control_unlock_door(event);
            break;

        case ACCESS_EVENT_ACCESS_DENIED:
            access_control_deny_access(event);
            break;

        case ACCESS_EVENT_TAMPER_ALERT:
            access_control_tamper_alert(event);
            break;

        case ACCESS_EVENT_SYSTEM_ERROR:
            access_control_system_error(event);
            break;

        default:
            break;
    }
}


/******************************************************************************
 * Local Functions
 ******************************************************************************/

/******************************************************************************
 * Callback Functions
 ******************************************************************************/
