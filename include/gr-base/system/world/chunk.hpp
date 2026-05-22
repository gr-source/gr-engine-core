#pragma once

#include "gr-base/ecs/default.hpp"
#include <cstdint>

#include <gr-math/vector2.hpp>

typedef uint32_t ChunkID;

#define INVALID_CHUNK (ChunkID)-1

#define CHUNK_RENDER_DISTANCE 8

#define CHUNK_SIZEI 16

typedef struct StreamChunk
{
    char* name;
    Vector2 position;
    bool dirty;

    Handle* objects;
} StreamChunk;

