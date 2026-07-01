#pragma once

#include "gr-base/ecs/componentRegistry.hpp"
#include "gr-base/ecs/default.hpp"

#include "gr-base/ecs/chunk.hpp"

#include <vector>

typedef struct ArchetypeLayout
{
    size_t capacity = 0;
    size_t stride = 0;

    size_t* offsets = nullptr;

    struct Graphy
    {
        size_t capacity = 0;

        ArchetypeID* removeEdges = nullptr;
        ArchetypeID* addEdges = nullptr;

        void grow(size_t new_capacity);
    } graphy;
} ArchetypeLayout;

struct ArchetypeStorage
{
    ArchetypeLayout* layout = nullptr;

    Signature mask = INVALID_SIGNATURE;

    ChunkID freeChunk = INVALID_CHUNK;
    ChunkID* freeList = nullptr;

    size_t chunkCapacity = 0;
    Chunk* chunks = nullptr;

    size_t entityCount = 0;

    // std::vector<ArchetypeID> m_removeEdges;
    // std::vector<ArchetypeID> m_addEdges;

    ArchetypeID addEdges(TypeID typeID);

    engine::ecs::detail::location allocate(Entity entity);

    uint8_t* get(engine::ecs::detail::location loc, const ComponentInfo* info) const;

    template <typename T>
    inline T* getRaw(ChunkID chunkID) const
    {
        return reinterpret_cast<T*>(getRaw(chunkID, ComponentRegistry::GetTypeID<T>()));
    }

    uint8_t* getRaw(ChunkID chunkID, TypeID typeID) const;

    Entity deallocate(engine::ecs::detail::location loc);

    void grow();

    void addFreeID(size_t oldCapacity);
};



