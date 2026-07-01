#pragma once

#include "gr-base/ecs/default.hpp"

#include <cstddef>
#include <cstdint>

#define CHUNK_SIZE (16 * 1024)

typedef struct Chunk
{
    size_t count = 0;

    Entity* entities = nullptr;
    uint8_t* array = nullptr;
} Chunk;


