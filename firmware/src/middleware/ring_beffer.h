/**************************************************************************//**
 * @file      ring_buffer.h
 * @brief     Ring buffer implementation for ESE516 with Doxygen-style comments
 * @author    Tony Yan
 * @date      2026-09-10
 ******************************************************************************/

#ifndef RING_BUFFER_H
#define RING_BUFFER_H 

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdint.h>

/******************************************************************************
 * Defines
 ******************************************************************************/
#define RING_BUFFER_SIZE 128  // Define the size of the ring buffer

/******************************************************************************
 * Structures and Enumerations
 ******************************************************************************/
typedef struct {
    uint8_t* buffer;
    size_t capacity;
    size_t head;
    size_t tail;
    size_t count;
} ring_buffer_t;

/******************************************************************************
 * Global Function Declaration
 ******************************************************************************/
/**
 * @brief Initializes a ring buffer structure. This function sets the head, tail, and count of the ring buffer to zero, effectively resetting it to an empty state.
 *
 * @param[in] rb Pointer to the ring buffer structure to be initialized.
 * @param[in] buffer Pointer to the buffer array to be used for the ring buffer.
 * @param[in] capacity The maximum number of elements the ring buffer can hold.
 * @param[out] None
 *
 * @return Returns true if the ring buffer was successfully initialized, false if the parameters are invalid (e.g., NULL pointer or zero capacity).
 */
bool ring_buffer_init(ring_buffer_t* rb, uint8_t* buffer, size_t capacity);

/**
 * @brief Pushes a byte of data into the ring buffer. If the buffer is full, the function will return false, indicating that the push operation failed.
 *
 * @param[in] rb Pointer to the ring buffer structure to be initialized.
 * @param[in] data The byte of data to be pushed into the ring buffer.
 * @param[out] None
 *
 * @return Returns true if the data was successfully pushed into the buffer, false if the buffer is full.
 */
bool ring_buffer_put(ring_buffer_t *rb, uint8_t data);

/**
 * @brief Pops a byte of data from the ring buffer. If the buffer is empty, the function will return false, indicating that the pop operation failed.
 *
 * @param[in] rb Pointer to the ring buffer structure.
 * @param[out] data Pointer to the location where the popped data will be stored.
 *
 * @return Returns true if data was successfully popped from the buffer, false if the buffer is empty.
 */
bool ring_buffer_get(ring_buffer_t *rb, uint8_t *data);

/**
 * @brief Checks if the ring buffer is empty.
 *
 * @param[in] rb Pointer to the ring buffer structure.
 *
 * @return Returns true if the buffer is empty, false otherwise.
 */
bool ring_buffer_is_empty(const ring_buffer_t *rb);

/**
 * @brief Checks if the ring buffer is full.
 *
 * @param[in] rb Pointer to the ring buffer structure.
 *
 * @return Returns true if the buffer is full, false otherwise.
 */
bool ring_buffer_is_full(const ring_buffer_t *rb);

#ifdef __cplusplus
}
#endif

#endif  //RING_BUFFER_H