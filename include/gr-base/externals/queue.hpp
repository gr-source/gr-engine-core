#pragma once

#include <cstddef>
#include <cstdint>

template <typename T, size_t N>
struct Queue
{
    uint32_t head = 0;
    uint32_t tail = 0;

    T data[N];


    bool push(const T& e)
    {
        uint32_t next = (head + 1) & (N - 1);

        if (next == tail)
            return false;

        data[head] = e;
        head = next;

        return true;
    }

    bool pop(T& e)
    {
        if (tail == head)
            return false;

        e = data[tail];
        tail = (tail + 1) & (N - 1);

        return true;
    }
};




