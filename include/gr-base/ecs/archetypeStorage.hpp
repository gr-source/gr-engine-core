#pragma once

#include "gr-base/ecs/default.hpp"

#include <vector>

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

    Signature m_mask;

    size_t m_capacity;

    size_t m_stride;

    uint8_t* m_buffer;

    ComponentID m_count;
    
    std::vector<ComponentID> m_entityToIndex;

    std::vector<Entity> m_denseEntities;

public:
    ArchetypeStorage(Signature mask, TypeID typeID);

    std::vector<ArchetypeStorage*> m_removeEdges;
    std::vector<ArchetypeStorage*> m_addEdges;

    ArchetypeStorage* addEdges(TypeID typeID);

    inline Signature GetMask() const
    {
        return m_mask;
    }

    void LinkAdd(TypeID type, ArchetypeStorage* dst);

    ComponentID Allocate(Entity e);

    void Deallocate(Entity e);

    ComponentID GetIndexOf(Entity e) const;

    Entity GetEntity(ComponentID id) const;

    void* Get(Entity entity, TypeID typeID);

    uint8_t* GetRaw(TypeID typeID) const;

    template <typename T>
    T* GetRaw() const;

    inline ComponentID Count() const
    {
        return m_count;
    }

    std::vector<TypeID> GetTypes() const;

private:
    void resize();
};

