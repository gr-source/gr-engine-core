#pragma once

#include "gr-base/ecs/default.hpp"
#include <cstdint>

#include <gr-math/vector2.hpp>

typedef uint32_t CellID;

#define INVALID_CELL (CellID)-1

#define CELL_RENDER_DISTANCE 8

#define CELL_SIZE 16

typedef struct StreamChunk
{
    char* name;
    Entity* entities;
    Vector2 position;
} StreamChunk;

