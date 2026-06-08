#pragma once

#include "gr-base/ecs/default.hpp"

#include <cstddef>
#include <cstdint>

#define CHUNK_SIZE (16 * 1024)

struct Chunk
{
    size_t capacity = 0;
    size_t count = 0;

    Entity* entities = nullptr;
    uint8_t* data = nullptr;
};


