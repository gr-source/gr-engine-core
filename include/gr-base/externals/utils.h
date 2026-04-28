#ifndef UTILS_H
#define UTILS_H

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

// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function#FNV-1a_hash
inline uint64_t fnv1_hash(const char* str)
{
    uint64_t hash = 14695981039346656037UL;

    char c = '\0';
    while ((c = *str++))
    {
        hash ^= (uint64_t)(uint8_t)(c);
        hash *= 1099511628211UL;
    }
    return hash;
}

inline uint64_t fnv1_hash_buff(const uint8_t* buff, uint32_t length)
{
    uint64_t hash = 14695981039346656037UL;

    for (uint32_t i=0;i<length;i++)
    {
        hash ^= (uint64_t)(buff[i]);
        hash *= 1099511628211UL;
    }
    return hash;
}

#endif // !UTILS_H
