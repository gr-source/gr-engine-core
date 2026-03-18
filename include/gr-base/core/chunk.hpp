#pragma once

#include "gr-base/ecs/default.hpp"

#include "gr-base/pool/poolObject.hpp"

#include <gr-math/gmath.hpp>

#include <vector>

extern "C"
{
#include "gr-base/externals/vector.h"
#include "gr-base/externals/array.h"
}

#define INVALID_CHUNK INVALID_POOL

#define CHUNK_RENDER_DISTANCE 8
#define CHUNK_SIZE 32

typedef PoolID ChunkID;

typedef struct StreamChunk
{
    Vector2 position;

    char* name;

    bool dirty;

    GameObjectHandle* objects;
} StreamChunk;

class Chunk
{
    Vector2 position;
public:
    Chunk(const std::string& name, const Vector2& position);

    std::string name;

    std::vector<GameObjectHandle> objects;

    bool dirty;

    void render();
};



