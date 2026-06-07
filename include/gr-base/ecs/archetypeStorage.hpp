#pragma once

#include "gr-base/ecs/default.hpp"
#include "gr-base/ecs/type.hpp"

#include <vector>

#define CHUNK_SIZE (16 * 1024)

class ArchetypeStorage
{
    std::vector<size_t> m_typeidtoarrayoffset;

public:
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

