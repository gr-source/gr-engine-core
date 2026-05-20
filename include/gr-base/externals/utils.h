#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <stdint.h>

// https://en.wikipedia.org/wiki/Bitwise_operation
inline uint32_t make_version(uint16_t minor, uint16_t major)
{
    return minor | major << 16;
}

inline uint16_t get_minor(uint32_t version)
{
    return version & 0xFFFF;
}

inline uint16_t get_major(uint32_t version)
{
    return (version >> 16) & 0xFFFF;
}

#endif // !UTILS_H
