#pragma once

#include "gr-base/ecs/default.hpp"
#include "gr-base/ecs/signature.hpp"

#define align_up(value, alignment) (((value) + ((alignment) - 1)) & ~((alignment) - 1))   

namespace engine::ecs::detail
{
typedef struct location
{
    ChunkID chunk = INVALID_CHUNK;
    uint32_t row = 0;
} location;

struct records
{
    Signature mask = INVALID_SIGNATURE;

    ArchetypeID archetype = 0;

    location location;
};

struct layout
{
    size_t capacity = 0;
    size_t stride = 0;
    size_t alignment = 0;

    size_t payloadSize = 0;

    size_t numOffsets = 0;
    size_t* offsets = nullptr;
};
}

