#pragma once

#include "gr-base/ecs/componentRegistry.hpp"
#include "gr-base/ecs/detail.hpp"

namespace engine::ecs::dynamic
{
    struct alignas(16) Chunk
    {
        size_t count = 0;
    };

    struct archetype : engine::ecs::detail::archetype_base<Chunk*>
    {
        uint8_t* base = nullptr;

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
}

