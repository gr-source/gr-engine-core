#include "gr-base/ecs/archetypeStorage.hpp"

#include "gr-base/core/logger.hpp"
#include "gr-base/ecs/type.hpp"

#include <cassert>
#include <cstdlib>
#include <cstring>

ArchetypeStorage::ArchetypeStorage(Signature mask, TypeID typeID) : m_mask(mask), m_buffer(nullptr), m_capacity(0), m_stride(0), entityCount(0)
{
    if (typeID != INVALID_TYPE)
    {
        m_removeEdges.resize(typeID + 1, INVALID_ARCHETYPE);
        m_addEdges.resize(typeID + 1, INVALID_ARCHETYPE);
    }

    while (mask)
    {
        TypeID typeID = __builtin_ctzll(mask);

        auto info = Type::GetTypeInfoByID(typeID);
        
        if (typeID >= m_typeidtoarrayoffset.size())
            m_typeidtoarrayoffset.resize(typeID + 1, 0);

        m_stride += info->stride;

        mask &= (mask - 1);
    }
}

ArchetypeStorage::ArchetypeStorage(ArchetypeStorage&& other) noexcept
{
    m_typeidtoarrayoffset = std::move(other.m_typeidtoarrayoffset);

    m_mask = other.m_mask;

    m_capacity = other.m_capacity;

    m_stride = other.m_stride;

    m_buffer = other.m_buffer;
    other.m_buffer = nullptr;

    entityCount = other.entityCount;

    slotToEntity = std::move(other.slotToEntity);
    
    m_removeEdges = std::move(other.m_removeEdges);
    m_addEdges = std::move(other.m_addEdges);
}

ArchetypeStorage::~ArchetypeStorage()
{
    // logger::trace("[ ArchetypeStorage ] Destroy '0x%lX' free='0x%lX'", this, m_buffer);

    if (m_buffer != nullptr)
        free(m_buffer);
}

ArchetypeID ArchetypeStorage::addEdges(TypeID typeID)
{
    if (typeID >= m_addEdges.size())
        m_addEdges.resize(typeID + 1, INVALID_ARCHETYPE);

    if (typeID >= m_removeEdges.size())
        m_removeEdges.resize(typeID + 1, INVALID_ARCHETYPE);

    return m_addEdges[typeID];
}

ComponentID ArchetypeStorage::Allocate(Entity entity)
{
    if (entityCount >= m_capacity)
        resize();

    ComponentID id = entityCount++;
    assert(id < m_capacity);

    slotToEntity[id] = entity;

    return id;
}

bool ArchetypeStorage::Deallocate(ComponentID removeComponentID)
{
    assert(removeComponentID < entityCount);

    ComponentID lastComponentID = --entityCount;
    if (removeComponentID != lastComponentID)
    {
        Signature mask = m_mask;
        while (mask)
        {
            TypeID typeID = __builtin_ctzll(mask);

            auto type = Type::GetTypeInfoByID(typeID);

            auto offset = m_typeidtoarrayoffset[typeID];

            uint8_t* array = m_buffer + offset;

            void* lastComponent = (void*)(array + (lastComponentID * type->stride));
            void* currentComponent = (void*)(array + (removeComponentID * type->stride));

            ((void(*)(void*, const void*))(type->move))(currentComponent, lastComponent);
            // memcpy(currentComponent, lastComponent, chunk.stride);

            mask &= (mask - 1);
        }

        // m_entityToIndex[handle_get_first(m_denseEntities[lastComponentID])] = removeComponentID;

        slotToEntity[removeComponentID] = slotToEntity[lastComponentID];

        return true;
    }

    return false;
}

void* ArchetypeStorage::Get(ComponentID id, const TypeInfo* type)
{
    if (id == INVALID_COMPONENT)
        return nullptr;

    uint8_t* array = m_buffer + m_typeidtoarrayoffset[type->index];

    return array + (id * type->stride);
}

uint8_t* ArchetypeStorage::GetRaw(TypeID typeID) const
{
    return m_buffer + m_typeidtoarrayoffset[typeID];
}

std::vector<TypeID> ArchetypeStorage::GetTypes() const
{
    std::vector<TypeID> vector;

    uint64_t mask = m_mask;
    while (mask)
    {
        vector.push_back(__builtin_ctzll(mask));

        mask &= (mask - 1);
    }

    return vector;
}

void ArchetypeStorage::resize()
{
    ComponentID oldCapacity = m_capacity;

    if (m_capacity)
    {
        m_capacity *= 2;

        uint8_t* newBuffer = (uint8_t*)malloc(m_capacity * m_stride);

        size_t newOffset = 0;
        uint64_t mask = m_mask;
        while (mask)
        {
            auto type = Type::GetTypeInfoByID(__builtin_ctzll(mask));

            auto oldOffset = m_typeidtoarrayoffset[type->index];

            uint8_t* lastArray = m_buffer + oldOffset;
            uint8_t* newArray = newBuffer + newOffset;

            if (type->globalCpy)
            {
                memcpy(newArray, lastArray, oldCapacity * type->stride);
            } else
            {
                uint8_t* oldComponent = lastArray;
                uint8_t* newComponent = newArray;

                for (ComponentID i=0;i<entityCount;i++)
                {
                    ((void (*)(void*, const void*))(type->move))(newComponent, oldComponent);

                    oldComponent += type->stride;
                    newComponent += type->stride;
                }
            }

            mask &= (mask - 1);

            // chunk.offset = newOffset;

            newOffset += type->stride * m_capacity;
        }

        free(m_buffer);
        m_buffer = newBuffer;
    } else
    {
        m_capacity = 2;

        m_buffer = (uint8_t*)malloc(m_capacity * m_stride);
    }

    // logger::trace("[ ArchetypeStorage ] Buffer of '0x%lX' resized '%ld' to '%ld'.", this, oldCapacity, m_capacity);

    slotToEntity.resize(m_capacity, INVALID_ENTITY);

    size_t offset = 0;
    uint64_t mask = m_mask;
    while (mask)
    {
        auto type = Type::GetTypeInfoByID(__builtin_ctzll(mask));

        m_typeidtoarrayoffset[type->index] = offset;

        offset += type->stride * m_capacity;

        mask &= (mask - 1);
    }
}



