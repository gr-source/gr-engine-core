#include "gr-base/ecs/archetypeStorage.hpp"

#include "gr-base/core/logger.hpp"
#include "gr-base/ecs/type.hpp"

#include <iostream>
#include <cstdlib>
#include <cstring>

ArchetypeStorage::ArchetypeStorage(Signature mask, TypeID typeID) : m_mask(mask), m_buffer(nullptr), m_capacity(0), m_stride(0), m_count(0)
{
    m_buffer = (uint8_t*)malloc(sizeof(int));

    logger::trace("[ ArchetypeStorage ] Create '0x%lX'", this);

    if (typeID != INVALID_TYPE)
    {
        m_removeEdges.resize(typeID + 1, INVALID_ARCHETYPE);
        m_addEdges.resize(typeID + 1, INVALID_ARCHETYPE);
    }

    while (mask)
    {
        TypeID typeID = __builtin_ctzll(mask);

        auto info = Type::GetTypeInfoByID(typeID);
        
        if (typeID >= m_typeToChunk.size())
            m_typeToChunk.resize(typeID + 1, INVALID_TYPE);

        auto& chunk = m_chunkInfoMap.emplace_back();
        chunk.stride = info->stride;
        chunk.offset = static_cast<uint32_t>(m_stride);
        chunk.typeID = typeID;

        m_typeToChunk[typeID] = static_cast<ChunkID>(m_chunkInfoMap.size() - 1);

        m_stride += info->stride;

        mask &= (mask - 1);
    }
}

ArchetypeStorage::ArchetypeStorage(ArchetypeStorage&& other) noexcept
{
    logger::trace("[ ArchetypeStorage ] Move '0x%lX' To '0x%lX'", &other, this);

    m_chunkInfoMap = std::move(other.m_chunkInfoMap);
    m_typeToChunk  = std::move(other.m_typeToChunk);

    m_mask = other.m_mask;

    m_capacity = other.m_capacity;

    m_stride = other.m_stride;

    m_buffer = other.m_buffer;
    other.m_buffer = nullptr;

    m_count = other.m_count;
    
    m_entityToIndex = std::move(other.m_entityToIndex);

    m_denseEntities = std::move(other.m_denseEntities);

    m_removeEdges = std::move(other.m_removeEdges);
    m_addEdges = std::move(other.m_addEdges);
}

ArchetypeStorage::~ArchetypeStorage()
{
    logger::trace("[ ArchetypeStorage ] Destroy '0x%lX' free='0x%lX'", this, m_buffer);

    if (m_buffer != nullptr)
        free(m_buffer);
}

ArcheTypeID ArchetypeStorage::addEdges(TypeID typeID)
{
    if (typeID >= m_addEdges.size())
        m_addEdges.resize(typeID + 1, INVALID_ARCHETYPE);

    if (typeID >= m_removeEdges.size())
        m_removeEdges.resize(typeID + 1, INVALID_ARCHETYPE);

    return m_addEdges[typeID];
}

/*
void ArchetypeStorage::LinkAdd(TypeID type, ArchetypeStorage* dst)
{
    m_addEdges[type] = dst;
    dst->m_removeEdges[type] = this;
}
*/

ComponentID ArchetypeStorage::Allocate(Entity e)
{
    if (m_count >= m_capacity)
        resize();

    EntityID entityID = handle_get_first(e);

    if (entityID >= m_entityToIndex.size())
        m_entityToIndex.resize(entityID + 1, INVALID_COMPONENT);

    ComponentID id = m_count++;

    m_entityToIndex[entityID] = id;
    m_denseEntities[id] = e;

    return id;
}

void ArchetypeStorage::Deallocate(Entity e)
{
    EntityID entityID = handle_get_first(e);

    if (entityID >= m_entityToIndex.size())
        m_entityToIndex.resize(entityID + 1, INVALID_COMPONENT);

    if (m_entityToIndex[entityID] == INVALID_COMPONENT)
        return;

    ComponentID removeComponentID = m_entityToIndex[entityID];
    ComponentID lastComponentID = --m_count;
    if (removeComponentID != lastComponentID)
    {
        for (auto& chunk : m_chunkInfoMap)
        {
            auto type = Type::GetTypeInfoByID(chunk.typeID);

            uint8_t* array = m_buffer + chunk.offset;

            void* lastComponent = (void*)(array + (lastComponentID * chunk.stride));
            void* currentComponent = (void*)(array + (removeComponentID * chunk.stride));

            ((void(*)(void*, const void*))(type->move))(currentComponent, lastComponent);
            // memcpy(currentComponent, lastComponent, chunk.stride);
        }

        m_entityToIndex[handle_get_first(m_denseEntities[lastComponentID])] = removeComponentID;

        m_denseEntities[removeComponentID] = m_denseEntities[lastComponentID];
    }
}

ComponentID ArchetypeStorage::GetIndexOf(Entity e) const
{
    uint32_t entityID = handle_get_first(e);
    if (entityID >= m_entityToIndex.size())
        return INVALID_COMPONENT;
    return m_entityToIndex[entityID];
}

Entity ArchetypeStorage::GetEntity(ComponentID id) const
{
    if (id >= m_denseEntities.size())
        return INVALID_ENTITY;
    return m_denseEntities[id];
}

void* ArchetypeStorage::Get(Entity entity, TypeID typeID)
{
    ComponentID id = GetIndexOf(entity);
    if (id == INVALID_COMPONENT)
        return nullptr;

    ChunkInfo& info = m_chunkInfoMap[m_typeToChunk[typeID]];

    uint8_t* array = m_buffer + info.offset;

    return array + (id * info.stride);
}

uint8_t* ArchetypeStorage::GetRaw(TypeID typeID) const
{
    auto& chunk = m_chunkInfoMap[m_typeToChunk[typeID]];

    return m_buffer + chunk.offset;
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

            auto& chunk = m_chunkInfoMap[m_typeToChunk[type->index]];

            size_t oldOffset = chunk.offset;

            uint8_t* lastArray = m_buffer + oldOffset;
            uint8_t* newArray = newBuffer + newOffset;

            if (type->globalCpy)
            {
                memcpy(newArray, lastArray, oldCapacity * chunk.stride);
            } else
            {
                uint8_t* oldComponent = lastArray;
                uint8_t* newComponent = newArray;

                for (ComponentID i=0;i<m_count;i++)
                {
                    ((void (*)(void*, const void*))(type->move))(newComponent, oldComponent);

                    oldComponent += type->stride;
                    newComponent += type->stride;
                }
            }

            mask &= (mask - 1);

            // chunk.offset = newOffset;

            newOffset += chunk.stride * m_capacity;
        }

        free(m_buffer);
        m_buffer = newBuffer;
    } else
    {
        m_capacity = 2;

        m_buffer = (uint8_t*)malloc(m_capacity * m_stride);
    }

    logger::trace("[ ArchetypeStorage ] Buffer of '0x%lX' resized '%ld' to '%ld'.", this, oldCapacity, m_capacity);

    m_denseEntities.resize(m_capacity, INVALID_ENTITY);

    size_t offset = 0;
    uint64_t mask = m_mask;
    while (mask)
    {
        TypeID typeID = __builtin_ctzll(mask);

        auto& chunk = m_chunkInfoMap[m_typeToChunk[typeID]];
        chunk.offset = offset;

        offset += chunk.stride * m_capacity;

        mask &= (mask - 1);
    }
}



