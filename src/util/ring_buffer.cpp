#include "ring_buffer.h"
#include <stdlib.h>
#include <string.h>

int ring_buffer_t::init(size_t capacity, size_t element_size)
{
    this->buffer = malloc(capacity * element_size);
    if (!this->buffer)
        return -1;
    this->capacity = capacity;
    this->element_size = element_size;
    this->write_index = 0;
    this->read_index = 0;

    return 0;
}

void ring_buffer_t::free_buffer()
{
    free(this->buffer);
}

void ring_buffer_t::reset()
{
    this->write_index = 0;
    this->read_index = 0;
}


int ring_buffer_t::is_empty()
{
    return this->write_index == this->read_index;
}

int ring_buffer_t::is_full()
{
    return ((this->write_index + 1) % this->capacity) == this->read_index;
}

int ring_buffer_t::push(const void *item)
{
    if (is_full())
        return -1;
    memcpy((char *)this->buffer + this->write_index * this->element_size, item, this->element_size);
    this->write_index = (this->write_index + 1) % this->capacity;
    return 0;
}

int ring_buffer_t::pop(void *item)
{
    if (is_empty())
    {
        return -1;
    }
    if (item != nullptr)
    {
        memcpy(item, (char *)this->buffer + this->read_index * this->element_size, this->element_size);
    }
    this->read_index = (this->read_index + 1) % this->capacity;

    return 1;
}

void ring_buffer_t::handle(std::function<void(void *)> callback)
{
    size_t buffer_write = this->write_index;
    size_t buffer_read = this->read_index;
    size_t buffer_max = this->capacity;
    size_t buffer_num = (buffer_write + buffer_max - buffer_read) % buffer_max;

    for (size_t i = 0; i < buffer_num; i++)
    {
        size_t buffer_read = this->read_index;

        void *target = this->buffer + this->element_size * buffer_read;
        callback(target);

        this->read_index = (this->read_index + 1) % this->capacity;
    }
}

// int ring_buffer_t::get_available()
// {
//     size_t buffer_write = this->write_index;
//     size_t buffer_read = this->read_index;
//     size_t buffer_max = this->capacity;

//     size_t buffer_num = (buffer_write + buffer_max - buffer_read) % buffer_max;

//     return buffer_num;
// }

// void *ring_buffer_t::get_one()
// {
//     size_t buffer_read = this->read_index;

//     return this->buffer + this->element_size * buffer_read;
// }
