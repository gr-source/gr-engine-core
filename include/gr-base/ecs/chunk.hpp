#pragma once

#include "gr-base/ecs/default.hpp"

#include <cstddef>
#include <cstdint>

#define CHUNK_SIZE (64 * 1024)

typedef struct Chunk
{
    size_t count = 0;

    Entity* entities = nullptr;

    uint8_t* base = nullptr;
} Chunk;


