#include "gr-base/ecs/archetypeStorage.hpp"

#include "gr-base/core/logger.hpp"

#include <cassert>
#include <cstdlib>
#include <cstring>

ArchetypeStorage::ArchetypeStorage(Signature _mask, TypeID typeID) :
    mask(_mask),
    typeOffset_capacity(0),
    typeOffsets(nullptr),

    freeID(INVALID_CHUNK),
    freeList(nullptr),

    chunk_capacity(0),
    chunks(nullptr),

    component_capacity(0),
    component_stride(0)
{

    if (typeID != INVALID_TYPE)
    {
        m_removeEdges.resize(typeID + 1, INVALID_ARCHETYPE);
        m_addEdges.resize(typeID + 1, INVALID_ARCHETYPE);
    }

    Signature temp = mask;
    while (temp)
    {
        auto info = ComponentRegistry::GetComponentInfoByID(__builtin_ctzll(temp));

        component_stride += info->stride;

        temp &= (temp - 1);
    }

    if (component_stride)
        component_capacity = CHUNK_SIZE / component_stride;

    size_t offset = 0;
    temp = mask;
    while (temp)
    {
        auto info = ComponentRegistry::GetComponentInfoByID(__builtin_ctzll(temp));
        
        while (info->index >= typeOffset_capacity)
            grow_type();

        typeOffsets[info->index] = offset;

        offset += info->stride * component_capacity;

        temp &= (temp - 1);
    }
}


ArchetypeStorage::ArchetypeStorage(ArchetypeStorage&& other) noexcept :
    mask(other.mask)
{
    typeOffset_capacity = other.typeOffset_capacity;

    typeOffsets = other.typeOffsets;
    other.typeOffsets = nullptr;

    component_capacity = other.component_capacity;

    component_stride = other.component_stride;

    freeID = other.freeID;

    freeList = other.freeList;
    other.freeList = nullptr;

    chunk_capacity = other.chunk_capacity;

    chunks = other.chunks;
    other.chunks = nullptr;

    stats = other.stats;

    m_removeEdges = std::move(other.m_removeEdges);
    m_addEdges = std::move(other.m_addEdges);
}

ArchetypeStorage::~ArchetypeStorage()
{
    if (typeOffsets != nullptr)
        free(typeOffsets);

    if (freeList != nullptr)
        free(freeList);

    if (chunks != nullptr)
        free(chunks);
}

ArchetypeID ArchetypeStorage::addEdges(TypeID typeID)
{
    if (typeID >= m_addEdges.size())
        m_addEdges.resize(typeID + 1, INVALID_ARCHETYPE);

    if (typeID >= m_removeEdges.size())
        m_removeEdges.resize(typeID + 1, INVALID_ARCHETYPE);

    return m_addEdges[typeID];
}

EntityLocation ArchetypeStorage::allocate(Entity entity)
{
    if (freeID == INVALID_CHUNK)
        grow();

    ChunkID chunkID = freeID;
    assert(chunkID < chunk_capacity);

    Chunk& chunk = chunks[chunkID];

    uint32_t row = static_cast<uint32_t>(chunk.count++);
    assert(row < chunk.capacity);

    chunk.entities[row] = entity;

    if (chunk.count >= chunk.capacity)
    {
        freeID = freeList[chunkID];
        freeList[chunkID] = INVALID_CHUNK;
    }

    stats.entityCount++;

    return {chunkID, row};
}

uint8_t* ArchetypeStorage::get(EntityLocation loc, const ComponentInfo* info) const
{
    assert(info->index < typeOffset_capacity);

    assert(loc.chunk < chunk_capacity);

    Chunk& chunk = chunks[loc.chunk];
    assert(chunk.count >= loc.row);

    uint8_t* array = chunk.data + typeOffsets[info->index];

    return array + (loc.row * info->stride);
}

uint8_t* ArchetypeStorage::getRaw(ChunkID chunkID, TypeID typeID) const
{
    assert(typeID < typeOffset_capacity);
    assert(chunkID < chunk_capacity);

    Chunk& chunk = chunks[chunkID];

    return chunk.data + typeOffsets[typeID];
}

Entity ArchetypeStorage::deallocate(EntityLocation loc)
{
    assert(loc.chunk < chunk_capacity);

    Chunk& chunk = chunks[loc.chunk];
    assert(chunk.count > 0);
    assert(loc.row < chunk.count);

    bool wasFull = (chunk.count == chunk.capacity);

    uint32_t dst_row = loc.row;
    uint32_t src_row = --chunk.count;

    Entity moved = INVALID_ENTITY;

    if (dst_row != src_row)
    {
        Signature temp = mask;
        while (temp)
        {
            auto info = ComponentRegistry::GetComponentInfoByID(__builtin_ctzll(temp));

            uint8_t* array = chunk.data + typeOffsets[info->index];

            uint8_t* src = array + (info->stride * src_row);
            uint8_t* dst = array + (info->stride * dst_row);

            ((void (*)(uint8_t*, const uint8_t*))(info->move))(dst, src);
            
            temp &= (temp - 1);
        }

        moved = chunk.entities[src_row];

        chunk.entities[dst_row] = moved;
    }

    Signature temp = mask;
    while (temp)
    {
        auto info = ComponentRegistry::GetComponentInfoByID(__builtin_ctzll(temp));

        uint8_t* array = chunk.data + typeOffsets[info->index];

        uint8_t* src = array + (info->stride * src_row);

        ((void (*)(uint8_t*))(info->destructor))(src);
        
        temp &= (temp - 1);
    }

    if (wasFull)
    {
        freeList[loc.chunk] = freeID;
        freeID = loc.chunk;
    }

    stats.entityCount--;

    return moved;
}

void ArchetypeStorage::grow()
{
    size_t oldCapacity = chunk_capacity;

    chunk_capacity = chunk_capacity > 0 ? chunk_capacity * 2 : 2;

    freeList = (ChunkID*)realloc(freeList, chunk_capacity * sizeof(ChunkID));
    chunks = (Chunk*)realloc(chunks, chunk_capacity * sizeof(Chunk));

    for (size_t i=oldCapacity;i<chunk_capacity;i++)
    {
        Chunk& chunk = chunks[i];
        chunk.capacity = component_capacity;
        chunk.count = 0;

        chunk.entities = component_capacity > 0 ? ((Entity*)malloc(component_capacity * sizeof(Entity))) : nullptr;
        chunk.data = component_capacity > 0 ? ((uint8_t*)malloc(component_capacity * component_stride)) : nullptr;
    }

    addFreeID(oldCapacity);
}

void ArchetypeStorage::addFreeID(size_t oldCapacity)
{
    size_t capacity = chunk_capacity - 1;
    for (size_t i=oldCapacity;i<capacity;i++)
        freeList[i] = static_cast<ChunkID>(i + 1);

    freeList[capacity] = freeID;
    freeID = static_cast<ChunkID>(oldCapacity);
}

void ArchetypeStorage::grow_type()
{
    typeOffset_capacity = typeOffset_capacity > 0 ? typeOffset_capacity * 2 : 2;

    typeOffsets = (size_t*)realloc(typeOffsets, typeOffset_capacity * sizeof(size_t));
}

/*
ArchetypeStorage::ArchetypeStorage(Signature mask, TypeID typeID) :
    m_mask(mask),
    m_buffer(nullptr),
    m_capacity(0),
    m_stride(0),
    m_count(0)
{
    if (typeID != INVALID_TYPE)
    {
        m_removeEdges.resize(typeID + 1, INVALID_ARCHETYPE);
        m_addEdges.resize(typeID + 1, INVALID_ARCHETYPE);
    }

    while (mask)
    {
        auto info = Type::GetComponentInfoByID(__builtin_ctzll(mask));
        
        if (info->index >= m_typeidtoarrayoffset.size())
            m_typeidtoarrayoffset.resize(info->index + 1, 0);

        m_typeidtoarrayoffset[info->index] = m_stride;

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

    m_count = other.m_count;

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
    if (m_count >= m_capacity)
        resize();

    ComponentID id = static_cast<ComponentID>(m_count++);

    slotToEntity[id] = entity;

    return id;
}

bool ArchetypeStorage::Deallocate(ComponentID removeComponentID)
{
    assert(removeComponentID < entityCount);

    ComponentID lastComponentID = static_cast<ComponentID>(--m_count);
    if (removeComponentID != lastComponentID)
    {
        Signature mask = m_mask;
        while (mask)
        {
            TypeID typeID = __builtin_ctzll(mask);

            auto type = Type::GetComponentInfoByID(typeID);

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

void* ArchetypeStorage::Get(ComponentID id, const ComponentInfo* type)
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
            auto type = Type::GetComponentInfoByID(__builtin_ctzll(mask));

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

                for (size_t i=0;i<m_count;i++)
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
        auto type = Type::GetComponentInfoByID(__builtin_ctzll(mask));

        m_typeidtoarrayoffset[type->index] = offset;

        // allocate(type, m_capacity);

        offset += type->stride * m_capacity;

        mask &= (mask - 1);
    }
}


#include <iostream>
uint8_t* ArchetypeStorage::allocate(const ComponentInfo* info, size_t capacity)
{
    std::cout << "[ ========== " << info->name << " ========== ]"<< std::endl;

    size_t bufferlen = info->stride * capacity;

    std::cout << "stride: " << bufferlen << std::endl;

    uint8_t* buffer = nullptr;
    if (info->alignment > alignof(std::max_align_t))
    {
        size_t aligned_size = (bufferlen + info->alignment - 1) & ~(info->alignment - 1);

        std::cout << "stride aligned: " << aligned_size << std::endl;

        // buffer = aligned_alloc(buf, size_t size)
    } else
    {}

    return buffer;
}
*/


