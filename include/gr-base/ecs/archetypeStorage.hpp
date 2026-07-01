#pragma once

#include "gr-base/ecs/componentRegistry.hpp"
#include "gr-base/ecs/default.hpp"

#include "gr-base/ecs/chunk.hpp"
#include "gr-base/ecs/detail.hpp"

#include <vector>

/*
struct ArchetypeStorage
{
    engine::ecs::detail::layout* layout = nullptr;

    Signature mask = INVALID_SIGNATURE;

    ChunkID freeChunk = INVALID_CHUNK;
    ChunkID* freeList = nullptr;

    size_t chunkCapacity = 0;
    Chunk* chunks = nullptr;

    size_t entityCount = 0;

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
*/


