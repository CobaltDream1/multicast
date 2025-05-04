#pragma once
#include <unordered_map>
#include <stack>
#include <vector>
#include <cstdio>
#include <cstdint>

template <typename T>
class buffer_pool_t
{
    T *buffer;
    bool *occupied;
    std::stack<size_t> available;
    size_t capacity;

public:
    buffer_pool_t()
    {
    }

    ~buffer_pool_t()
    {
        delete[] buffer;
        delete[] occupied;
    }

    void init(size_t capacity)
    {
        this->capacity = capacity;
        this->buffer = new T[capacity];
        this->occupied = new bool[capacity]{false};
        for (int i = capacity; i >= 0; i--)
            this->available.push(i);
    }

    
    static buffer_pool_t<T> &get_instance()
    {
        static buffer_pool_t<T> instance;

        return instance;
    }

    int allocate()
    {
        if (this->available.empty())
        {
            printf("No available space in buffer.\n");
            return -1;
        }

        size_t pos = this->available.top();
        this->available.pop();
        this->occupied[pos] = true;

        return static_cast<int>(pos);
    }

    T *find(int index)
    {
        if (index < 0 || static_cast<size_t>(index) >= this->capacity)
            return nullptr;
        if (!this->occupied[index])
            return nullptr;
        return &buffer[index];
    }

    void remove(int index)
    {
        if (index < 0 || static_cast<size_t>(index) >= this->capacity)
            return;
        if (!occupied[index])
            return;

        occupied[index] = false;
        available.push(index);
    }
};
