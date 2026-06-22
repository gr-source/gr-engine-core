#pragma once

#include "gr-base/ecs/default.hpp"
#include "gr-base/externals/hash.hpp"
#include "gr-base/managers/default.hpp"
#include <cstdint>

#include <gr-math/vector2.hpp>

typedef uint32_t CellID;

#define INVALID_CELL (CellID)-1

#define TERRAIN_LOAD_DISTANCE 8

#define TERRAIN_UNLOAD_DISTANCE TERRAIN_LOAD_DISTANCE * 2

#define TERRAIN_CHUNK_SIZE 32

#define TERRAIN_CHUNK_BITS 5

#define TERRAIN_NUM_LOD_LEVEL 4

typedef uint64_t ChunkCoord;

enum class ChunkState
{
    INACTIVE,
    LOADING,
    LOADED
};


typedef struct BoundingSphere
{
    Vector3 center;
    float radius;
} BoundingSphere;

typedef struct MeshTerrainLOD
{
    MeshHandle mesh;
    BoundingSphere bounds;
} MeshTerrainLOD;

struct StreamChunk
{
    ChunkCoord coord;

    ChunkState state;

    MaterialHandle material;

    MeshTerrainLOD meshLOD[TERRAIN_NUM_LOD_LEVEL];

    Matrix4x4 transform;
};


