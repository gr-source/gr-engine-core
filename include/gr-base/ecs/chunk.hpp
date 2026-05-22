#pragma once

#include <cstddef>
#include <cstdint>

#define CHUNK_SIZE (16 * 1024)

struct Chunk
{
    size_t capacity;
    size_t count;

    uint8_t* memory;
};


