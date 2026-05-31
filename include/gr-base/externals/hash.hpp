#pragma once

#include <cstddef>
#include <cstdint>

using Hash64 = uint64_t;

#define INVALID_HASH64 (Hash64)-1

namespace hash
{
    // https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function#FNV-1a_hash
    Hash64 fnv1_str(const char* str);

    Hash64 fnv1_buff(const uint8_t* buff, size_t length);

    uint32_t crc32_str(const char* str);

    uint32_t crc32_buff(const uint8_t* data, size_t length);
};


