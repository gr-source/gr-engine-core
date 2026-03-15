#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function#FNV-1a_hash
inline uint64_t GetHash(const char* str)
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

#endif // !UTILS_H
