#pragma once

#include <cstddef>

namespace engine::memory
{
    void* alloc(size_t size);

    void  free(void* ptr);

    void* realloc(void* ptr, size_t oldSize, size_t newSize);
}
