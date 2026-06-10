#pragma once

#include <string_view>
#include <cstddef>
#include <cstdint>

using Hash64 = uint64_t;

#define INVALID_HASH64 (Hash64)-1

namespace engine::hash
{
    // https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function#FNV-1a_hash
    constexpr Hash64 fnv1_str(const std::string_view& str)
    {
        uint64_t hash = 14695981039346656037UL;

        for (size_t i=0;i<str.size();i++)
        {
            hash ^= (uint64_t)(uint8_t)(str[i]);
            hash *= 1099511628211UL;
        }
        return hash;
    }

    Hash64 fnv1_buff(const uint8_t* buff, size_t length);

    uint32_t crc32_str(const char* str);

    uint32_t crc32_buff(const uint8_t* data, size_t length);
};


