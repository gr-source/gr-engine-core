#pragma once

#include <cstddef>

#define MALLOC(s, align) engine::memory::alloc(s)

#define FREE(p) engine::memory::free(p)

#define REALLOC(p) engine::memory::realloc(p, size)

namespace engine::memory
{
    void* alloc(size_t size);

    void free(void* ptr);

    void* realloc(void* ptr, size_t oldSize, size_t newSize);
}



