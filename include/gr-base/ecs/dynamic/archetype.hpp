#pragma once

#include "gr-base/ecs/componentRegistry.hpp"
#include "gr-base/ecs/detail.hpp"

namespace engine::ecs::dynamic
{
    struct alignas(16) Chunk
    {
        size_t count = 0;

        Entity* entities = nullptr;

        uint8_t* base = nullptr;
    };

    struct archetype
    {
        engine::ecs::detail::layout* layout = nullptr;

        Signature mask = INVALID_SIGNATURE;

        ChunkID freeChunk = INVALID_CHUNK;
        ChunkID* freeList = nullptr;

        size_t entityCount = 0;

        size_t chunkCapacity = 0;
        Chunk** chunks = nullptr;

        ArchetypeID addEdges(TypeID typeID);

        engine::ecs::detail::location allocate(Entity entity);

        template <typename T>
        inline T* get(engine::ecs::detail::location loc) const
        {
            return getRaw<T>(loc.chunk) + loc.row;
        }

        template <typename T>
        inline T* getRaw(ChunkID chunkID) const
        {
            return reinterpret_cast<T*>(getRaw(chunkID, ComponentRegistry::GetTypeID<T>()));
        }

        void* getRaw(ChunkID chunkID, TypeID typeID) const;

        Entity deallocate(engine::ecs::detail::location loc);

        void grow();

        Chunk* create() const;

        void addFreeID(size_t oldCapacity);
    };
}

