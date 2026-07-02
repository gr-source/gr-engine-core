#include "gr-base/ecs/dynamic/archetype.hpp"
#include <cstdlib>

#include <iostream>

namespace engine::ecs::dynamic
{
ArchetypeID archetype::addEdges(TypeID typeID)
{
    if (typeID >= layout->graphy.capacity)
        layout->graphy.grow(typeID + 1);

    return layout->graphy.addEdges[typeID];
}

engine::ecs::detail::location archetype::allocate(Entity entity)
{
    if (freeChunk == INVALID_CHUNK)
        grow();

    ChunkID chunkID = freeChunk;
    assert(chunkID < chunkCapacity);

    Chunk* chunk = chunks[chunkID];

    Entity* entities = reinterpret_cast<Entity*>(
        reinterpret_cast<uint8_t*>(chunk) + sizeof(Chunk)
    );

    uint32_t row = static_cast<uint32_t>(chunk->count++);
    assert(row < layout->capacity);

    entities[row] = entity;

    if (chunk->count >= layout->capacity)
    {
        freeChunk = freeList[chunkID];
        freeList[chunkID] = INVALID_CHUNK;
    }

    entityCount++;

    return {chunkID, row};
}

uint8_t* archetype::get(engine::ecs::detail::location loc, const ComponentInfo* info) const
{
    return getRaw(loc.chunk, info->index) + (loc.row * info->stride);
}

uint8_t* archetype::getRaw(ChunkID chunkID, TypeID typeID) const
{
    assert(chunkID < chunkCapacity);

    Chunk* chunk = chunks[chunkID];
    assert(chunk != nullptr);

    Entity* entities = reinterpret_cast<Entity*>(
        reinterpret_cast<uint8_t*>(chunk) + sizeof(Chunk)
    );

    uint8_t* ptr = reinterpret_cast<uint8_t*>(
        entities + layout->capacity
    );

    uint64_t p = reinterpret_cast<uint64_t>(ptr);

    uint64_t aligned = (p + layout->alignment - 1) & ~(layout->alignment - 1);

    uint8_t* buffer = reinterpret_cast<uint8_t*>(aligned);

    size_t offset = layout->offsets[typeID];

    assert((offset % layout->alignment) == 0);

    uint8_t* array = buffer + offset;

    return array;
}

Entity archetype::deallocate(engine::ecs::detail::location loc)
{
    assert(loc.chunk < chunkCapacity);

    Chunk* chunk = chunks[loc.chunk];

    Entity* entities = reinterpret_cast<Entity*>(
        reinterpret_cast<uint8_t*>(chunk) + sizeof(Chunk)
    );

    uint8_t* ptr = reinterpret_cast<uint8_t*>(
        entities + layout->capacity
    );

    uint64_t p = reinterpret_cast<uint64_t>(ptr);

    uint64_t aligned = (p + layout->alignment - 1) & ~(layout->alignment - 1);

    uint8_t* buffer = reinterpret_cast<uint8_t*>(aligned);

    bool wasFull = (chunk->count == layout->capacity);

    uint32_t dst_row = loc.row;
    uint32_t src_row = --chunk->count;

    Entity moved = INVALID_ENTITY;


    if (dst_row != src_row)
    {
        Signature temp = mask;
        while (temp)
        {
            auto info = ComponentRegistry::GetComponentByID(__builtin_ctzll(temp));

            uint8_t* array = buffer + layout->offsets[info->index];

            void* src = array + (info->stride * src_row);
            void* dst = array + (info->stride * dst_row);

            info->move(dst, src);
            
            temp &= (temp - 1);
        }

        moved = entities[src_row];

        entities[dst_row] = moved;
    }

    Signature temp = mask;
    while (temp)
    {
        auto info = ComponentRegistry::GetComponentByID(__builtin_ctzll(temp));

        uint8_t* array = buffer + layout->offsets[info->index];

        void* src = array + (info->stride * src_row);

        info->destructor(src);
        
        temp &= (temp - 1);
    }

    if (wasFull)
    {
        freeList[loc.chunk] = freeChunk;
        freeChunk = loc.chunk;
    }

    entityCount--;

    return moved;
}

void archetype::grow()
{
    size_t oldCapacity = chunkCapacity;

    chunkCapacity = chunkCapacity > 0 ? chunkCapacity * 2 : 2;

    freeList = (ChunkID*)realloc(freeList, chunkCapacity * sizeof(ChunkID));
    chunks = (Chunk**)realloc(chunks, chunkCapacity * sizeof(Chunk*));

    size_t alignedSize = (layout->payloadSize + layout->alignment - 1) & ~(layout->alignment - 1);

    for (size_t i=oldCapacity;i<chunkCapacity;i++)
    {
        Chunk* ptr = (Chunk*)std::aligned_alloc(layout->alignment, alignedSize);;
        ptr->count = 0;

        chunks[i] = ptr;
    }

    addFreeID(oldCapacity);
}

void archetype::addFreeID(size_t oldCapacity)
{
    size_t capacity = chunkCapacity - 1;
    for (size_t i=oldCapacity;i<capacity;i++)
        freeList[i] = static_cast<ChunkID>(i + 1);

    freeList[capacity] = freeChunk;
    freeChunk = static_cast<ChunkID>(oldCapacity);
}
}
