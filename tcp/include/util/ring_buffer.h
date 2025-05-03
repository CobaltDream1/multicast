#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stddef.h>

typedef struct
{
    void *buffer;
    size_t capacity;
    size_t element_size;
    size_t write_index;
    size_t read_index;
    int full;
} ring_buffer_t;

typedef int (*ring_buffer_handler)(void *);

int ring_buffer_init(ring_buffer_t *rb, size_t capacity, size_t element_size);
void ring_buffer_free(ring_buffer_t *rb);

int ring_buffer_push(ring_buffer_t *rb, const void *item);
int ring_buffer_pop(ring_buffer_t *rb, void* item);

// 放在一起用
int ring_buffer_get_available(ring_buffer_t *rb);
void* ring_buffer_get_one(ring_buffer_t *rb);

int ring_buffer_is_empty(const ring_buffer_t *rb);
int ring_buffer_is_full(const ring_buffer_t *rb);

#endif

