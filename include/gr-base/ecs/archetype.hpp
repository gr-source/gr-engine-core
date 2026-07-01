#pragma once

#include "type_traits.hpp"
#include "default.hpp"

#include <cstdlib>

#define INVALIDU32 UINT32_MAX
#define INVALIDU64 UINT64_MAX

namespace engine::ecs
{
template <size_t N, typename T>
struct StackBase
{
    T data[N];
};

template <size_t N, typename... T>
struct MultiStack : StackBase<N, T>...
{
};

typedef struct EntityLocation
{
    uint32_t chunk;
    uint32_t row;
} EntityLocation;

template <typename... Components>
struct Archetype
{
    static constexpr size_t CAPACITY = engine::type_traits::capacity<Components...>;

    template <typename T>
    using Component = StackBase<CAPACITY, T>;

    typedef struct Chunk
    {
        size_t count = 0;

        Entity entities[CAPACITY];

        MultiStack<CAPACITY, Components...> array;
    } Chunk;

    using ChunkID = uint32_t;

    // ========================================
    ArchetypeLayout* layout = nullptr;

    Signature mask = INVALID_SIGNATURE;

    ChunkID freeChunk = INVALID_CHUNK;
    ChunkID* freeList = nullptr;

    size_t chunkCapacity = 0;
    Chunk* chunks = nullptr;

    size_t entityCount = 0;

    // ========================================

    ~Archetype()
    {
        size_t chunk_count = CAPACITY > 0 ? (entityCount + CAPACITY - 1) / CAPACITY : 0;

        for (size_t i=0;i<chunk_count;i++)
        {
            Chunk& chunk = chunks[i];

            for (size_t j=0;j<chunk.count;j++)
            {
                (apply_destructor(
                       static_cast<Component<Components>&>(chunk.array).data[j]), ...);
            }
        }

        if (chunks != nullptr)
            free(chunks);

        if (freeList != nullptr)
            free(freeList);
    }

    template <typename T>
    void apply_construtor(T& dst, const T& src)
    {
        new (&dst) T(src);
    }

    template <typename T>
    void apply_construtor_move(T& dst, const T& src)
    {
        dst = std::move(src);

        apply_destructor(src);

        // new (&dst) T(std::move(src));
    }

    template <typename T>
    void apply_destructor(T& dst)
    {
        dst.~T();
    }

    EntityLocation Add(Entity entity, const Components&... components)
    {
        if (freeChunk == INVALIDU32)
            grow();

        ChunkID chunkID = freeChunk;
        assert(chunkID < capacity);

        Chunk& chunk = chunks[chunkID];
        assert(chunk.count < CAPACITY);

        uint32_t row = static_cast<uint32_t>(chunk.count++);
        chunk.entities[row] = entity;

        if (chunk.count >= CAPACITY)
        {
            freeChunk = freeList[chunkID];
            freeList[chunkID] = INVALIDU32;
        }

        (apply_construtor(static_cast<StackBase<CAPACITY, Components>&>(chunk.array).data[row], components), ...);

        entityCount++;

        return { chunkID, row };
    }

    Entity Del(EntityLocation loc)
    {
        assert(loc.chunk < capacity);

        Chunk& chunk = chunks[loc.chunk];
        assert(loc.row < chunk.count);

        bool wasFull = (chunk.count == CAPACITY);

        uint32_t dst_row = loc.row;
        uint32_t src_row = --chunk.count;

        Entity moved = INVALIDU64;

        if (dst_row != src_row)
        {
            (apply_construtor_move(
                   static_cast<Component<Components>&>(chunk.array).data[dst_row],
                   static_cast<Component<Components>&>(chunk.array).data[src_row]), ...);

            moved = chunk.entities[src_row];

            chunk.entities[dst_row] = moved;
        }

        (apply_destructor(
               static_cast<Component<Components>&>(chunk.array).data[src_row]), ...);

        if (wasFull)
        {
            freeList[loc.chunk] = freeChunk;
            freeChunk = loc.chunk;
        }

        entityCount--;

        return moved;
    }

    template <typename T>
    T& Get(EntityLocation loc)
    {
        assert(loc.chunk < capacity);
        
        Chunk& chunk = chunks[loc.chunk];
        assert(chunk.count < CAPACITY);

        Component<T>& components = chunk.array;

        return components.data[loc.row];
    }

    void grow()
    {
        size_t oldCapacity = chunkCapacity;

        chunkCapacity = chunkCapacity > 0 ? chunkCapacity * 2 : 2;

        chunks = (Chunk*)realloc(chunks, chunkCapacity * sizeof(Chunk));
        freeList = (ChunkID*)realloc(freeList, chunkCapacity * sizeof(ChunkID));

        for (size_t i=oldCapacity;i<chunkCapacity;i++)
            new (chunks + i) Chunk();

        addFreeList(oldCapacity);
    }

    void addFreeList(size_t oldCapacity)
    {
        for (size_t i=oldCapacity;i<chunkCapacity - 1;i++)
            freeList[i] = static_cast<ChunkID>(i + 1);

        freeList[chunkCapacity - 1] = freeChunk;
        freeChunk = static_cast<ChunkID>(oldCapacity);
    }

    template <typename Func>
    void each(Func&& func)
    {
        size_t chunk_count = CAPACITY > 0 ? (entityCount + CAPACITY - 1) / CAPACITY : 0;

        for (size_t i=0;i<chunk_count;i++)
        {
            Chunk& chunk = chunks[i];

            func(chunk.count, chunk.entities, static_cast<StackBase<CAPACITY, Components>&>(chunk.array).data...);
        }
    }
};

}

#undef INVALIDU32
#undef INVALIDU64


