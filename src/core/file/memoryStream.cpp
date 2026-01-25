#include "gr-base/core/file/memoryStream.hpp"

#include <string.h>

MemoryStream::MemoryStream(uint8_t* data, size_t size)
    : begin(data), current(data), end(data + size), capacity(1024) {}

MemoryStream::MemoryStream(size_t size)
{
    begin = (uint8_t*)malloc(size);
    current = begin;
    end = begin + size;
    capacity = size;
}

bool MemoryStream::write(const void* dst, size_t size)
{
    if (current + size > end)
    {
        size_t used = current - begin;
        size_t newCapacity = std::max(capacity * 2, used + size);
        uint8_t* newBuffer = (uint8_t*)malloc(newCapacity);
        memcpy(newBuffer, begin, used);
        free(begin);
        begin = newBuffer;
        current = begin + used;
        end = begin + newCapacity;
        capacity = newCapacity;
    }

    memcpy(current, dst, size);
    current += size;
    return true;
}

bool MemoryStream::read(void* dst, size_t size)
{
    if (current + size > end)
        return false;
    memcpy(dst, current, size);
    current += size;
    return true;
}

bool MemoryStream::seekg(size_t pos)
{
    if (begin + pos > end)
        return false;
    current = begin + pos;
    return true;
}

bool MemoryStream::seekp(size_t pos)
{
    return seekg(pos);
}

size_t MemoryStream::tellg()
{
    return static_cast<size_t>(current - begin);
}

size_t MemoryStream::tellp()
{
    return tellg();
}

bool MemoryStream::eof() const
{
    return current >= end;
}

void* MemoryStream::data() const
{
    return begin;
}

void MemoryStream::close()
{
    free(begin);
}

