#include "gr-base/ecs/archetype.hpp"

#include "gr-base/ecs/componentRegistry.hpp"

#include <cassert>
#include <cstdlib>

namespace engine::ecs
{
/*

*/

detail::location archetype::allocate(Entity entity)
{
    uint32_t id = static_cast<uint32_t>(entityCount++);

    uint32_t chunkIndex = id / layout->capacity;
    uint32_t row = id & (layout->capacity - 1);

    if (chunkIndex >= chunkCapacity)
        grow();

    Chunk* chunk = chunks[chunkIndex];
    if (chunk->count == 0 && chunkIndex >= chunkCount)
        chunkCount++;

    chunk->count++;

    chunk->entities[row] = entity;

    return { chunkIndex, row };
}

Entity archetype::deallocate(detail::location loc)
{
    uint32_t lastGlobal = static_cast<uint32_t>(--entityCount);

    uint32_t dstChunk = loc.chunk;
    uint32_t dstIndex = loc.row;

    uint32_t srcChunk = lastGlobal / layout->capacity;
    uint32_t srcIndex = lastGlobal & (layout->capacity - 1);

    Entity moved = INVALID_ENTITY;

    if (dstChunk != srcChunk || dstIndex != srcIndex)
    {
        Chunk* dst_chunk = chunks[dstChunk];
        Chunk* src_chunk = chunks[srcChunk];

        Signature temp = mask;
        while (temp)
        {
            auto info = ComponentRegistry::GetComponentByID(__builtin_ctzll(temp));
            if (info->stride > 0)
            {
                uint8_t* dst_array = dst_chunk->base + layout->offsets[info->index];
                uint8_t* src_array = src_chunk->base + layout->offsets[info->index];

                void* dst = dst_array + (info->stride * dstIndex);
                void* src = src_array + (info->stride * srcIndex);

                info->move(dst, src);
            }
            
            temp &= (temp - 1);
        }

        moved = src_chunk->entities[srcIndex];

        dst_chunk->entities[dstIndex] = moved;
    }

    Chunk* lastChunk = chunks[srcChunk];
    lastChunk->count--;

    if (lastChunk->count == 0 && srcChunk == chunkCount - 1)
        chunkCount--;

    return moved;
}

void* archetype::getRaw(ChunkID chunkID, TypeID typeID) const
{
    assert(chunkID < chunkCapacity);
    assert(typeID < layout->numOffsets);

    Chunk* chunk = chunks[chunkID];
    assert(chunk != nullptr);

    return static_cast<void*>(chunk->base + layout->offsets[typeID]);
}

void archetype::grow()
{
    size_t oldCapacity = chunkCapacity;

    chunkCapacity = chunkCapacity > 0 ? chunkCapacity * 2 : 2;

    freeList = (ChunkID*)realloc(freeList, chunkCapacity * sizeof(ChunkID));
    chunks = (Chunk**)realloc(chunks, chunkCapacity * sizeof(Chunk*));

    for (size_t i=oldCapacity;i<chunkCapacity;i++)
        chunks[i] = create();

    addFreeID(oldCapacity);
}


Chunk* archetype::create() const
{
    Chunk* chunk = (Chunk*)std::aligned_alloc(layout->alignment, layout->payloadSize);;
    new (chunk) Chunk;

    chunk->entities =
        reinterpret_cast<Entity*>(
            reinterpret_cast<uint8_t*>(chunk) + sizeof(Chunk)
        );

    size_t headerAndEntitiesSize =
        sizeof(Chunk) +
        (sizeof(Entity) * layout->capacity);

    size_t baseOffset = align_up(headerAndEntitiesSize, layout->alignment);

    chunk->base = reinterpret_cast<uint8_t*>(
        chunk
    ) + baseOffset;

    return chunk;
}

void archetype::addFreeID(size_t oldCapacity)
{
    size_t capacity = chunkCapacity - 1;
    for (size_t i=oldCapacity;i<capacity;i++)
        freeList[i] = static_cast<ChunkID>(i + 1);

    freeList[capacity] = freeChunk;
    freeChunk = static_cast<ChunkID>(oldCapacity);

    chunkCount++;
}

void graph::grow(size_t new_capacity)
{
    if (capacity >= new_capacity)
        return;

    size_t oldCapacity = capacity;

    capacity = new_capacity;

    removeEdges = (ArchetypeID*)realloc(removeEdges, capacity * sizeof(ArchetypeID));
    addEdges = (ArchetypeID*)realloc(addEdges, capacity * sizeof(ArchetypeID));

    for (size_t i=oldCapacity;i<capacity;i++)
    {
        removeEdges[i] = INVALID_ARCHETYPE;
        addEdges[i] = INVALID_ARCHETYPE;
    }
}
}


