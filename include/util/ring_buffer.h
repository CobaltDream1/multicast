#pragma once
#include <stddef.h>
#include <functional>

struct ring_buffer_t
{
    void *buffer;
    size_t capacity;
    size_t element_size;
    size_t write_index;
    size_t read_index;
    int full;

    int init(size_t capacity, size_t element_size);
    void reset();
    void free_buffer();
    
    int push(const void *item);
    int pop(void *item);
    
    // 一次性把所有的处理掉
    void handle(std::function<void(void *)> callback);
    
    int is_empty();
    int is_full();
};


