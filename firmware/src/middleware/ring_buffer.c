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
bool ring_buffer_init(ring_buffer_t* rb, uint8_t* buffer, size_t capacity)
{
    if(rb == NULL || buffer == NULL || capacity == 0)
    {
        return false; // Invalid parameters
    }

    rb -> buffer = buffer;
    rb -> capacity = capacity;
    rb -> head = 0;
    rb -> tail = 0;
    rb -> count = 0;
}

bool ring_buffer_put(ring_buffer_t *rb, uint8_t data)
{
    if(rb == NULL || rb->buffer == NULL)
    {
        return false; // Invalid ring buffer
    }

    if(ring_buffer_is_full(rb))
    {
        return false; // Buffer is full
    }

    rb->buffer[rb->head] = data;
    rb->head = (rb->head + 1) % rb->capacity;
    rb->count++;

    return true;
}

bool ring_buffer_get(ring_buffer_t *rb, uint8_t *data)
{
    if(rb == NULL || rb->buffer == NULL || data == NULL)
    {
        return false; // Invalid ring buffer or empty
    }

    if(ring_buffer_is_empty(rb)) {
        return false; // Buffer is empty
    }

    *data = rb->buffer[rb->tail];

    rb->tail = (rb->tail + 1) % rb->capacity;
    rb->count--;

    return true;
}

bool ring_buffer_is_empty(const ring_buffer_t *rb)
{
    if(rb == NULL) {
        return false;
    }

    return rb->count == 0;
}

bool ring_buffer_is_full(const ring_buffer_t* rb)
{
    if(rb == NULL) {
        return false;
    }

    return rb->count == rb->capacity;
}

/******************************************************************************
 * Local Functions
 ******************************************************************************/

/******************************************************************************
 * Callback Functions
 ******************************************************************************/
