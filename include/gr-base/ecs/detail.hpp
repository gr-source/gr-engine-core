#pragma once

#include "gr-base/ecs/default.hpp"
#include "gr-base/ecs/entityManager.hpp"

#include <unordered_map>
#include <vector>

#define align_up(value, alignment) (((value) + ((alignment) - 1)) & ~((alignment) - 1))   

namespace engine::ecs::detail
{
typedef struct location
{
    uint32_t chunk;
    uint32_t row;
} location;


typedef struct layout
{
    size_t capacity = 0;
    size_t stride = 0;
    size_t alignment = 0;

    size_t payloadSize = 0;

    size_t numOffsets = 0;
    size_t* offsets = nullptr;

    struct Graphy
    {
        size_t capacity = 0;

        ArchetypeID* removeEdges = nullptr;
        ArchetypeID* addEdges = nullptr;

        void grow(size_t new_capacity);
    } graphy;
} layout;

template <typename T>
struct archetype_base
{
    engine::ecs::detail::layout* layout = nullptr;

    Signature mask = INVALID_SIGNATURE;

    ChunkID freeChunk = INVALID_CHUNK;
    ChunkID* freeList = nullptr;

    size_t entityCount = 0;

    size_t chunkCapacity = 0;
    T* chunks = nullptr;
};

template <typename T>
struct world_base
{
    struct EntityRecord
    {
        uint32_t archetype;

        engine::ecs::detail::location location;
    };

    std::unordered_map<Signature, ArchetypeID> signatureToArchetype;

    engine::ecs::manager::entity entityManager;

    std::vector<EntityRecord> entityRecords;

    ArchetypeID m_emptyArchetypeStorage = INVALID_ARCHETYPE;

    T* m_archetypePool = nullptr;
    
    size_t m_capacity = 0;

    size_t m_count = 0;
};
}

