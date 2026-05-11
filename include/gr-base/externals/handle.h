#ifndef HANDLE_H
#define HANDLE_H

#include <stdint.h>

#define HANDLE_VERSION_SHIFT 32
#define HANDLE_INDEX_MASK    0x00000000FFFFFFFFULL

#define HANDLE_INVALID 0

typedef uint64_t Handle;

static inline Handle handle_create(uint32_t index, uint32_t version)
{
    return ((uint64_t)index) | ((uint64_t)version << HANDLE_VERSION_SHIFT);
}

static inline uint32_t handle_get_first(Handle h)
{
    return (uint32_t)(h & HANDLE_INDEX_MASK);
}

static inline uint32_t handle_get_second(Handle h)
{
    return (uint32_t)(h >> HANDLE_VERSION_SHIFT);
}

static inline int handle_equal(Handle lhs, Handle rhs)
{
    return lhs == rhs;
}

static inline int handle_is_valid(Handle h)
{
    return h != 0;
}

#endif // !HANDLE_H
