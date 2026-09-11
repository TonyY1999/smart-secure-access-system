/**************************************************************************//**
 * @file      ring_buffer.c
 * @brief     Ring buffer implementation for ESE516 with Doxygen-style comments
 * @author    Tony Yan
 * @date      2026-09-10
 ******************************************************************************/

/******************************************************************************
 * Includes
 ******************************************************************************/
#include "ring_buffer.h"

/******************************************************************************
 * Defines
 ******************************************************************************/

/******************************************************************************
 * Variables
 ******************************************************************************/

/******************************************************************************
 * Forward Declarations
 ******************************************************************************/

/******************************************************************************
 * Global Functions
 ******************************************************************************/
void ring_buffer_init(ring_buffer_t* rb)
{
    rb -> head = 0;
    rb -> tail = 0;
    rb -> count = 0;
}

bool ring_buffer_push(ring_buffer_t *rb, uint8_t data)
{
    if(rb == NULL || ring_buffer_is_full(rb))
    {
        return false; // Invalid ring buffer
    }

    rb -> buffer[rb -> head] = data;
    rb -> head = (rb -> head + 1) % RING_BUFFER_SIZE;
    rb -> count++;

    return true;
}

bool ring_buffer_pop(ring_buffer_t *rb, uint8_t *data)
{
    if(rb == NULL || rb -> count == 0)
    {
        return false; // Invalid ring buffer or empty
    }


    *data = rb -> buffer[rb -> tail];
    rb -> tail = (rb -> tail + 1) % RING_BUFFER_SIZE;
    rb -> count--;

    return true;
}

bool ring_buffer_is_empty(const ring_buffer_t *rb)
{
    if(rb == NULL) {
        return false;
    }

    return rb -> count == 0;
}

bool ring_buffer_is_full(const ring_buffer_t* rb)
{
    if(rb == NULL) {
        return false;
    }

    return rb -> count == RING_BUFFER_SIZE;
}

/******************************************************************************
 * Local Functions
 ******************************************************************************/

/******************************************************************************
 * Callback Functions
 ******************************************************************************/
