#pragma once

#include "gr-base/ecs/default.hpp"
#include "gr-base/externals/hash.hpp"
#include "gr-base/managers/default.hpp"
#include <cstdint>

#include <gr-math/vector2.hpp>

typedef uint32_t CellID;

#define INVALID_CELL (CellID)-1

#define LOAD_DISTANCE 8

#define UNLOAD_DISTANCE 16

#define TERRAIN_CHUNK_SIZE 32

struct ChunkCoord
{
    int x;
    int z;

    bool operator==(const ChunkCoord& c) const
    {
        return x == c.x && z == c.z;
    }
};

enum class ChunkState
{
    Loading,
    Loaded,
    Unloading
};

struct StreamChunk
{
    ChunkCoord coord;

    Hash64 hash;

    Vector2 position;

    ChunkState state;

    MaterialHandle material;

    MeshHandle mesh;

    Entity* entities;

    Matrix4x4 transform;
};

struct ChunkCoordHash
{
    uint64_t operator()(const ChunkCoord& c) const
    {
        return (uint64_t(uint32_t(c.x)) << 32)
             | uint32_t(c.z);
    }
};

