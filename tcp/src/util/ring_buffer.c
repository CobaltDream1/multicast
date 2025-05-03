#include "ring_buffer.h"
#include <stdlib.h>
#include <string.h>

int ring_buffer_init(ring_buffer_t *rb, size_t capacity, size_t element_size)
{
    rb->buffer = malloc(capacity * element_size);
    if (!rb->buffer)
        return -1;
    rb->capacity = capacity;
    rb->element_size = element_size;
    rb->write_index = 0;
    rb->read_index = 0;
    return 0;
}

void ring_buffer_free(ring_buffer_t *rb)
{
    if (rb != NULL)
    {
        free(rb->buffer);
        free(rb);
    }
}

int ring_buffer_is_empty(const ring_buffer_t *rb)
{
    return rb->write_index == rb->read_index;
}

int ring_buffer_is_full(const ring_buffer_t *rb)
{
    return ((rb->write_index + 1) % rb->capacity) == rb->read_index;
}

int ring_buffer_push(ring_buffer_t *rb, const void *item)
{
    if (ring_buffer_is_full(rb))
        return -1;
    memcpy((char *)rb->buffer + rb->write_index * rb->element_size, item, rb->element_size);
    rb->write_index = (rb->write_index + 1) % rb->capacity;
    return 0;
}

int ring_buffer_pop(ring_buffer_t *rb, void *item)
{
    if (ring_buffer_is_empty(rb))
    {
        return -1;
    }
    if (item != NULL)
    {
        memcpy(item, (char *)rb->buffer + rb->read_index * rb->element_size, rb->element_size);
    }
    rb->read_index = (rb->read_index + 1) % rb->capacity;
    return 0;
}

int ring_buffer_get_available(ring_buffer_t *rb)
{
    size_t buffer_write = rb->write_index;
    size_t buffer_read = rb->read_index;
    size_t buffer_max = rb->capacity;

    size_t buffer_num = (buffer_write + buffer_max - buffer_read) % buffer_max;

    return buffer_num;
}

void* ring_buffer_get_one(ring_buffer_t *rb)
{
    size_t buffer_read = rb->read_index;

    return &rb->buffer[buffer_read];
}
