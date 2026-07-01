#include "gr-base/ecs/dynamic/archetype.hpp"
#include <cstdlib>

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

    Chunk& chunk = chunks[chunkID];

    uint32_t row = static_cast<uint32_t>(chunk.count++);
    assert(row < chunk.capacity);

    chunk.entities[row] = entity;

    if (chunk.count >= layout->capacity)
    {
        freeChunk = freeList[chunkID];
        freeList[chunkID] = INVALID_CHUNK;
    }

    entityCount++;

    return {chunkID, row};
}

uint8_t* archetype::get(engine::ecs::detail::location loc, const ComponentInfo* info) const
{
    assert(info->index < typeOffset_capacity);

    assert(loc.chunk < chunkCapacity);

    Chunk& chunk = chunks[loc.chunk];
    assert(chunk.count >= loc.row);

    uint8_t* array = chunk.array + layout->offsets[info->index];

    return array + (loc.row * info->stride);
}

uint8_t* archetype::getRaw(ChunkID chunkID, TypeID typeID) const
{
    assert(typeID < typeOffset_capacity);
    assert(chunkID < chunkCapacity);

    Chunk& chunk = chunks[chunkID];

    return chunk.array + layout->offsets[typeID];
}

Entity archetype::deallocate(engine::ecs::detail::location loc)
{
    assert(loc.chunk < chunkCapacity);

    Chunk& chunk = chunks[loc.chunk];
    assert(chunk.count > 0);
    assert(loc.row < chunk.count);

    bool wasFull = (chunk.count == layout->capacity);

    uint32_t dst_row = loc.row;
    uint32_t src_row = --chunk.count;

    Entity moved = INVALID_ENTITY;

    if (dst_row != src_row)
    {
        Signature temp = mask;
        while (temp)
        {
            auto info = ComponentRegistry::GetComponentByID(__builtin_ctzll(temp));

            uint8_t* array = chunk.array + layout->offsets[info->index];

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
        auto info = ComponentRegistry::GetComponentByID(__builtin_ctzll(temp));

        uint8_t* array = chunk.array + layout->offsets[info->index];

        uint8_t* src = array + (info->stride * src_row);

        ((void (*)(uint8_t*))(info->destructor))(src);
        
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

    size_t chunkDataSize =
        layout->capacity * sizeof(Entity) +
        layout->capacity * layout->stride;

    freeList = (ChunkID*)realloc(freeList, chunkCapacity * sizeof(ChunkID));
    chunks = (Chunk*)realloc(chunks, chunkCapacity * (sizeof(Chunk) + chunkDataSize));

    for (size_t i=oldCapacity;i<chunkCapacity;i++)
    {
        Chunk& chunk = chunks[i];
        chunk.count = 0;

        chunk.entities = layout->capacity > 0 ? ((Entity*)malloc(layout->capacity * sizeof(Entity))) : nullptr;
        chunk.array = layout->capacity > 0 ? ((uint8_t*)malloc(layout->capacity * layout->stride)) : nullptr;
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
