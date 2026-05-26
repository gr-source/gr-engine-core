#pragma once

#include "gr-base/ecs/default.hpp"
#include "gr-base/ecs/type.hpp"

#include <vector>

#define CHUNK_SIZE (16 * 1024)

class ArchetypeStorage
{
    typedef struct ChunkInfo
    {
        size_t offset;
        size_t stride;
        TypeID typeID;
    } ChunkInfo;

    std::vector<ChunkInfo> m_chunkInfoMap;
    std::vector<ChunkID> m_typeToChunk;
    
    std::vector<ComponentID> m_entityToIndex;

    std::vector<Entity> m_denseEntities;

public:
    ArchetypeStorage(Signature mask, TypeID typeID);
    ArchetypeStorage(ArchetypeStorage&& other) noexcept;
    ~ArchetypeStorage();

    size_t m_capacity;

    size_t m_stride;

    uint8_t* m_buffer;

    ComponentID m_count;

    Signature m_mask;

    std::vector<ArcheTypeID> m_removeEdges;
    std::vector<ArcheTypeID> m_addEdges;

    ArcheTypeID addEdges(TypeID typeID);

    // void LinkAdd(TypeID type, ArcheTypeID dstID);

    ComponentID Allocate(Entity e);

    bool Deallocate(Entity e);

    ComponentID GetIndexOf(Entity e) const;

    Entity GetEntity(ComponentID id) const;

    const Entity* GetEntities() const;

    void* Get(Entity entity, TypeID typeID);

    uint8_t* GetRaw(TypeID typeID) const;

    template <typename T>
    inline T* GetRaw() const
    {
        return reinterpret_cast<T*>(GetRaw(Type::GetTypeID<T>()));
    }

    inline ComponentID Count() const
    {
        return m_count;
    }

    std::vector<TypeID> GetTypes() const;

private:
    void resize();
};

