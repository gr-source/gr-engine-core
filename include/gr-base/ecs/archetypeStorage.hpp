#pragma once

#include "gr-base/ecs/typeRegistry.hpp"
#include "gr-base/ecs/default.hpp"

#include "gr-base/ecs/chunk.hpp"

#include <vector>

struct ArchetypeStats
{
    size_t entityCount = 0;
};

struct EntityLocation
{
    uint32_t chunk;
    uint32_t row;
};

struct ArchetypeStorage
{
    const Signature mask = INVALID_SIGNATURE;

    size_t typeOffset_capacity = 0;

    size_t* typeOffsets = nullptr;

    size_t component_capacity = 0;

    size_t component_stride = 0;

    ChunkID freeID = INVALID_CHUNK;

    ChunkID* freeList = nullptr;

    size_t chunk_capacity = 0;

    Chunk* chunks = nullptr;

    ArchetypeStats stats;

    std::vector<ArchetypeID> m_removeEdges;
    std::vector<ArchetypeID> m_addEdges;

    ArchetypeStorage(Signature _mask, TypeID typeID);
    ArchetypeStorage(ArchetypeStorage&& other) noexcept;
    ~ArchetypeStorage();

    ArchetypeID addEdges(TypeID typeID);

    EntityLocation allocate(Entity entity);

    uint8_t* get(EntityLocation loc, const TypeInfo* info) const;

    template <typename T>
    inline T* getRaw(ChunkID chunkID) const
    {
        return reinterpret_cast<T*>(getRaw(chunkID, TypeRegistry::GetTypeID<T>()));
    }

    uint8_t* getRaw(ChunkID chunkID, TypeID typeID) const;

    Entity deallocate(EntityLocation loc);

    void grow();

    void addFreeID(size_t oldCapacity);

    void grow_type();
};

/*
class ArchetypeStorage
{
public:
    std::vector<size_t> m_typeidtoarrayoffset;

    ArchetypeStorage(Signature mask, TypeID typeID);
    ArchetypeStorage(ArchetypeStorage&& other) noexcept;
    ~ArchetypeStorage();

    std::vector<Entity> slotToEntity;

    size_t m_capacity;

    size_t m_count;

    size_t m_stride;

    uint8_t* m_buffer;

    Signature m_mask;

    std::vector<ArchetypeID> m_removeEdges;
    std::vector<ArchetypeID> m_addEdges;

    ArchetypeID addEdges(TypeID typeID);

    ComponentID Allocate(Entity entity);

    bool Deallocate(ComponentID id);

    void* Get(ComponentID id, const TypeInfo* type);

    uint8_t* GetRaw(TypeID typeID) const;

    template <typename T>
    inline T* GetRaw() const
    {
        return reinterpret_cast<T*>(GetRaw(Type::GetTypeID<T>()));
    }

    std::vector<TypeID> GetTypes() const;

private:
    void resize();

    uint8_t* allocate(const TypeInfo* info, size_t capacity);
};
*/


