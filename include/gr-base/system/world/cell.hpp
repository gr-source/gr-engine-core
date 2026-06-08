#pragma once

#include "gr-base/ecs/default.hpp"
#include <cstdint>

#include <gr-math/vector2.hpp>

typedef uint32_t CellID;

#define INVALID_CELL (CellID)-1

#define VIEW_DISTANCE 8

#define CELL_SIZE 16

struct ChunkCoord
{
    int x;
    int z;

    bool operator==(const ChunkCoord& c) const
    {
        return x == c.x && z == c.z;
    }
};

typedef struct StreamChunk
{
    Entity* entities;
    Vector2 position;
    ChunkCoord coord;
} StreamChunk;

struct ChunkCoordHash
{
    uint64_t operator()(const ChunkCoord& c) const
    {
        return (uint64_t(uint32_t(c.x)) << 32)
             | uint32_t(c.z);
    }
};

