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

#define TERRAIN_NUM_LOD_LEVEL 4

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
    Hash64 hash;

    ChunkState state;

    MaterialHandle material;

    MeshHandle mesh[TERRAIN_NUM_LOD_LEVEL];
    float radius[TERRAIN_NUM_LOD_LEVEL];

    Matrix4x4 transform;
};


