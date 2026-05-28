#include "gr-base/memory/memory.hpp"

#include <new>
#include <cstring>

namespace engine::memory
{
    constexpr size_t ALIGNMENT = 16;

    void* alloc(size_t size)
    {
        return ::operator new(size, std::align_val_t(ALIGNMENT));
    }

    void free(void* ptr)
    {
        ::operator delete(ptr, std::align_val_t(ALIGNMENT));
    }

    void* realloc(void* ptr, size_t oldSize, size_t newSize)
    {
        void* newPtr = alloc(newSize);

        if (ptr != nullptr)
        {
            std::memcpy(newPtr, ptr,
                oldSize < newSize ? oldSize : newSize);

            free(ptr);
        }

        return newPtr;
    }
}

