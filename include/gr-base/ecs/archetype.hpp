#pragma once

#include "gr-base/ecs/type_id.hpp"
#include "gr-base/ecs/detail.hpp"
#include "gr-base/ecs/chunk.hpp"

namespace engine::ecs
{
    struct archetype
    {
        detail::layout* layout = nullptr;

        Signature mask = INVALID_SIGNATURE;

        ChunkID freeChunk = INVALID_CHUNK;
        ChunkID* freeList = nullptr;

        size_t entityCount = 0;

        size_t chunkCapacity = 0;
        size_t chunkCount = 0;
        Chunk** chunks = nullptr;

        // ArchetypeID addEdges(TypeID typeID);

        detail::location allocate(Entity entity);

        Entity deallocate(detail::location loc);

        template <typename T>
        inline T* get(detail::location loc) const
        {
            return getRaw<T>(loc.chunk) + loc.row;
        }

        template <typename T>
        inline T* getRaw(ChunkID chunkID) const
        {
            return reinterpret_cast<T*>(getRaw(chunkID, GetComponentID<T>::id));
        }

        void* getRaw(ChunkID chunkID, TypeID typeID) const;

        void grow();

        Chunk* create() const;

        void addFreeID(size_t oldCapacity);
    };


    struct graph
    {
        size_t capacity = 0;

        ArchetypeID* removeEdges = nullptr;
        ArchetypeID* addEdges = nullptr;

        void grow(size_t new_capacity);
    };
}

