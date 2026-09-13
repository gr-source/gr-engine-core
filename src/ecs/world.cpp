#include "gr-base/ecs/world.hpp"

#include "gr-base/ecs/componentRegistry.hpp"
#include "gr-base/ecs/detail.hpp"
#include "gr-base/ecs/entityManager.hpp"

#include "gr-base/externals/handle.h"

#include <assert.h>
#include <cstdlib>
#include <cstring>

size_t bit_floor(size_t value)
{
    if (value == 0)
        return 0;

    size_t result = 1;

    while ((result << 1) <= value)
        result <<= 1;

    return result;
}

namespace engine::ecs
{

World::World()
{
    m_emptyArchetypeStorage = GetOrCreateArcheType(0);
}

World::~World()
{
    /*
    printf("\n========== Archetype Graph ==========\n");

    for (size_t id = 0; id < storage.count; ++id)
    {
        const auto& g = storage.graphs[id];
        const auto& arche = storage.archetypes[id];

        printf("\n[%zu] mask=0x%016llX\n",
               id,
               (unsigned long long)arche.mask);

        // Componentes do archetype
        printf("  Components: ");

        Signature mask = arche.mask;

        if (mask == 0)
        {
            printf("(empty)");
        }
        else
        {
            bool first = true;

            while (mask)
            {
                TypeID typeID = __builtin_ctzll(mask);

                if (!first)
                    printf(", ");

                printf("%s", ComponentRegistry::GetComponentByID(typeID)->name);

                first = false;
                mask &= mask - 1;
            }
        }

        printf("\n");

        bool hasEdges = false;

        for (size_t typeID = 0; typeID < g.capacity; ++typeID)
        {
            ArchetypeID add =
                g.addEdges
                    ? g.addEdges[typeID]
                    : INVALID_ARCHETYPE;

            ArchetypeID remove =
                g.removeEdges
                    ? g.removeEdges[typeID]
                    : INVALID_ARCHETYPE;

            if (add != INVALID_ARCHETYPE)
            {
                printf(
                    "  ADD    Type %zu -> Archetype %u\n",
                    typeID,
                    add
                );

                hasEdges = true;
            }

            if (remove != INVALID_ARCHETYPE)
            {
                printf(
                    "  REMOVE Type %zu -> Archetype %u\n",
                    typeID,
                    remove
                );

                hasEdges = true;
            }
        }

        if (!hasEdges)
            printf("  [no edges]\n");
    }

    printf("\n=====================================\n");
    */

    for (size_t i=0;i<storage.count;i++)
    {
        auto& arche = storage.archetypes[i];
        auto& graph = storage.graphs[i];

        if (graph.addEdges != nullptr)
            free(graph.addEdges);

        if (graph.removeEdges != nullptr)
            free(graph.removeEdges);

        auto layout = arche.layout;
        if (layout != nullptr)
        {
            if (layout->offsets != nullptr)
                free(layout->offsets);

            free(layout);
        }

        if (arche.freeList != nullptr)
            free(arche.freeList);

        if (arche.chunks != nullptr)
        {
            for (size_t i=0;i<arche.chunkCapacity;i++)
                free(arche.chunks[i]);
            free(arche.chunks);
        }
    }

    if (storage.archetypes != nullptr)
        free(storage.archetypes);

    if (storage.graphs != nullptr)
        free(storage.graphs);
}

Entity World::Create()
{
    return entityManager.create();
}

void World::Destroy(Entity entity)
{
    if (!entityManager.isAlive(entity))
        return;

    EntityID entityID = handle_get_first(entity);

    auto& rec = entityManager.records[entityID];

    assert(rec.archetype < storage.count);
    auto& arche = storage.archetypes[rec.archetype];

    Entity moved = INVALID_ENTITY;
    if ((moved = arche.deallocate(rec.location)) != INVALID_ENTITY)
        entityManager.records[handle_get_first(moved)].location = rec.location;

    entityManager.destroy(entity);
}

bool World::IsAlive(Entity entity) const
{
    return entityManager.isAlive(entity);
}

ArchetypeID World::GetOrCreateArcheType(const Signature mask)
{
    auto it = signatureToArchetype.find(mask);
    if (it != signatureToArchetype.end())
        return it->second;

    if (storage.count >= storage.capacity)
        resize();

    size_t numOffsets = 0;
    size_t stride = 0;
    size_t maxAlign = 1;

    maxAlign = std::max(maxAlign, alignof(Chunk));
    maxAlign = std::max(maxAlign, alignof(Entity));

    Signature temp = mask;
    while (temp)
    {
        TypeID typeID = __builtin_ctzll(temp);

        auto info = ComponentRegistry::GetComponentByID(typeID);
        if (info->stride > 0)
        {
            stride += info->stride;
            maxAlign = std::max(maxAlign, info->alignment);
        }

        if (typeID >= numOffsets)
            numOffsets = typeID + 1;
        
        temp &= (temp - 1);
    }

    auto layout = (detail::layout*)malloc(sizeof(detail::layout));
    new (layout) detail::layout();

    layout->numOffsets = numOffsets;
    layout->offsets = (size_t*)malloc(numOffsets * sizeof(size_t));

    layout->stride = stride;

    layout->capacity =
        stride > 0 ? (CHUNK_SIZE / stride) : 0;

    layout->capacity = bit_floor(layout->capacity);

    size_t localOffset = 0;
    {
        Signature temp = mask;
        while (temp)
        {
            TypeID index = __builtin_ctzll(temp);

            auto info = ComponentRegistry::GetComponentByID(index);
            if (info->stride > 0)
            {
                localOffset = align_up(localOffset, info->alignment);

                layout->offsets[index] = localOffset;

                localOffset += info->stride * layout->capacity;
            }

            temp &= (temp - 1);
        }
    }

    size_t headerAndEntitiesSize =
        sizeof(Chunk) +
        (sizeof(Entity) * layout->capacity);

    size_t baseOffset =
        align_up(headerAndEntitiesSize, maxAlign);

    size_t data_size =
        stride > 0 ?
            stride * layout->capacity : 0;

    layout->payloadSize =
        align_up(
                baseOffset + data_size,
                maxAlign
            );

    layout->alignment = maxAlign;

    ArchetypeID id = static_cast<ArchetypeID>(storage.count++);

    graph* g = storage.graphs + id;
    new (g) graph();

    if (numOffsets >= g->capacity)
        g->grow(numOffsets + 1);

    archetype* arche = GetArchetypeByID(id);
    new (arche) archetype();

    arche->layout = layout;
    arche->mask = mask;

    signatureToArchetype.emplace(mask, id);

    return id;
}

Signature World::GetMask(Entity e) const
{
    if (!entityManager.isAlive(e))
        return 0;

    EntityID entityID = handle_get_first(e);

    return entityManager.records[entityID].mask;
}

detail::records World::Allocate(Entity entity, Signature mask)
{
    uint32_t entityID = handle_get_first(entity);

    auto& currentRecord = entityManager.records[entityID];

    auto currentArchetype = GetArchetypeByID(currentRecord.archetype);
    if ((currentArchetype->mask & mask) == mask)
        return currentRecord;

    Signature newMask = currentArchetype->mask | mask;

    ArchetypeID archeID =
        GetOrCreateArcheType(newMask);

    auto newRecord = MoveEntity(currentRecord, archeID, entity);
    entityManager.records[entityID] = newRecord;

    return newRecord;
}

detail::records World::Allocate(Entity entity, TypeID type)
{
    Signature mask = (1ull << type);

    uint32_t entityID = handle_get_first(entity);

    auto& rec = entityManager.records[entityID];

    auto lastArcheType = GetArchetypeByID(rec.archetype);
    if ((lastArcheType->mask & mask) == mask)
        return rec;

    ArchetypeID dstID = AddEdges(rec.archetype, type);
    if (dstID == INVALID_ARCHETYPE)
    {
        Signature newMask = lastArcheType->mask | mask;

        dstID = GetOrCreateArcheType(newMask);

        LinkAdd(rec.archetype, type, dstID);
    }

    auto new_rec = MoveEntity(rec, dstID, entity);
    entityManager.records[entityID] = new_rec;

    return new_rec;
}


void World::AddComponentByMask(Entity entity, Signature requiredMask)
{
    if (!entityManager.isAlive(entity))
        return;

    uint32_t entityID = handle_get_first(entity);

    auto& current = entityManager.records[entityID];
    if (current.mask != INVALID_SIGNATURE)
    {
        Signature common = (current.mask & requiredMask);
        if (common == requiredMask)
            return;

        requiredMask &= ~common;
    }

    auto record = Allocate(entity, requiredMask);

    auto arche = GetArchetypeByID(record.archetype);

    while (requiredMask)
    {
        TypeID typeID = __builtin_ctzll(requiredMask);

        auto info = ComponentRegistry::GetComponentByID(typeID);

        if (info->stride > 0)
        {
            uint8_t* raw = (uint8_t*)arche->getRaw(record.location.chunk, typeID);

            void* component = (void*)(raw + record.location.row * info->stride);
            info->constructor(component, nullptr);
        }
        
        requiredMask &= (requiredMask - 1);
    }
}

void* World::AddComponentByID(Entity entity, TypeID typeID)
{
    if (!entityManager.isAlive(entity))
        return nullptr;

    auto info = ComponentRegistry::GetComponentByID(typeID);
    if (info == nullptr)
        return nullptr;

    uint32_t entityID = handle_get_first(entity);

    Signature requiredMask = (1ULL << typeID);

    auto& current = entityManager.records[entityID];
    if (
            current.mask != INVALID_SIGNATURE &&
            ((current.mask & requiredMask) == requiredMask)
       )
    {
        if (!info->stride)
            return nullptr;

        auto arche = GetArchetypeByID(current.archetype);

        uint8_t* raw = (uint8_t*)arche->getRaw(current.location.chunk, typeID);

        return (void*)(raw + current.location.row * info->stride);
    }

    auto record = Allocate(entity, typeID);

    if (!info->stride)
        return nullptr;

    auto arche = GetArchetypeByID(record.archetype);

    uint8_t* raw = (uint8_t*)arche->getRaw(record.location.chunk, typeID);

    void* component = (void*)(raw + record.location.row * info->stride);
    info->constructor(component, nullptr);

    return component;
}

void* World::GetComponentByID(Entity entity, TypeID type) const
{
    assert(entityManager.isAlive(entity));
    if (!entityManager.isAlive(entity))
        return nullptr;

    auto info = ComponentRegistry::GetComponentByID(type);
    assert(info != nullptr);
    if (info == nullptr)
        return nullptr;

    uint32_t entityID = handle_get_first(entity);

    auto& record = entityManager.records[entityID];
    assert(record.archetype != INVALID_ARCHETYPE);
    assert(record.mask != INVALID_SIGNATURE);

    Signature requiredMask = (1ull << type);

    assert((record.mask & requiredMask) == requiredMask);

    if (
            record.archetype == INVALID_ARCHETYPE ||
            record.mask == INVALID_SIGNATURE ||
            ((record.mask & requiredMask) != requiredMask)
        )
        return nullptr;

    if (!info->stride)
        return nullptr;

    auto arche = GetArchetypeByID(record.archetype);

    uint8_t* raw = (uint8_t*)arche->getRaw(record.location.chunk, type);

    return (void*)(raw + record.location.row * info->stride);
}

void World::DelComponentByID(Entity entity, TypeID typeID)
{
    DelComponentByMask(entity, (Signature)(1ull << typeID));
}

void World::DelComponentByMask(Entity entity, Signature requiredMask)
{
    if (!entityManager.isAlive(entity))
        return;

    uint32_t entityID = handle_get_first(entity);

    auto& rec = entityManager.records[entityID];

    auto src = GetArchetypeByID(rec.archetype);

    Signature oldMask = src->mask;
    Signature newMask = oldMask & ~requiredMask;

    auto dstID = GetOrCreateArcheType(newMask);
    if (dstID == rec.archetype)
        return;

    entityManager.records[entityID] =
        MoveEntity(rec, dstID, entity);
}

std::vector<TypeID> World::GetAllComponentType(Entity e) const
{
    std::vector<TypeID> vector;

    Signature temp = GetMask(e);
    while (temp)
    {
        vector.push_back(__builtin_ctzll(temp));

        temp &= (temp - 1);
    }

    return vector;
}

size_t World::GetEntityCount() const
{
    return entityManager.count;
}

engine::ecs::detail::records World::MoveEntity(engine::ecs::detail::records src_rec, ArchetypeID dstID, Entity e)
{
    // EntityID entityID = handle_get_first(e);

    archetype* src = GetArchetypeByID(src_rec.archetype);
    archetype* dst = GetArchetypeByID(dstID);

    // engine::ecs::detail::location src_loc = entityManager.records[entityID].location;
    engine::ecs::detail::location dst_loc = dst->allocate(e);

    Signature common = src->mask & dst->mask;
    while (common)
    {
        TypeID typeID = __builtin_ctzll(common);

        auto info = ComponentRegistry::GetComponentByID(typeID);
        if (info->stride > 0)
        {
            uint8_t* dst_raw = (uint8_t*)dst->getRaw(dst_loc.chunk, typeID);
            uint8_t* src_raw = (uint8_t*)src->getRaw(src_rec.location.chunk, typeID);

            void* dst_component = (void*)(dst_raw + dst_loc.row * info->stride);
            void* src_component = (void*)(src_raw + src_rec.location.row * info->stride);

            if (info->isTriviallyCopyable)
            {
                memcpy(
                    dst_component,
                    src_component,
                    info->stride
                );

            } else
            {
                ((void(*)(void*, const void*))(info->move))(dst_component, src_component);
            }

            // TODO: Isso deve crasha com objetos do stl.
            ((void (*)(void*))(info->destructor))(src_component);

        }

        common &= (common - 1);
    }

    Entity moved = INVALID_ENTITY;
    if (src_rec.location.chunk != INVALID_CHUNK && (moved = src->deallocate(src_rec.location)) != INVALID_ENTITY)
    {
        entityManager.records[handle_get_first(moved)].location = src_rec.location;
    }

    return
    {
        .mask = dst->mask,
        .archetype = dstID,
        .location = dst_loc
    };
}

archetype* World::GetArchetypeByID(ArchetypeID id) const
{
    if (id >= storage.count)
        return nullptr;
    return &storage.archetypes[id];
}

ArchetypeStorage& World::GetStorage()
{
    return storage;
}

void World::LinkAdd(ArchetypeID src, TypeID typeID, ArchetypeID dst)
{
    if (typeID >= storage.graphs[src].capacity)
        storage.graphs[src].grow(typeID + 1);

    if (typeID >= storage.graphs[dst].capacity)
        storage.graphs[dst].grow(typeID + 1);

    storage.graphs[src].addEdges[typeID] = dst;
    storage.graphs[dst].removeEdges[typeID] = src;
}

ArchetypeID World::AddEdges(ArchetypeID src, TypeID typeID)
{
    if (typeID >= storage.graphs[src].capacity)
        storage.graphs[src].grow(typeID + 1);

    return storage.graphs[src].addEdges[typeID];
}

void World::resize()
{
    storage.capacity = storage.capacity > 0 ? storage.capacity * 2 : 2;

    storage.archetypes =
        (archetype*)realloc(
            storage.archetypes,
            storage.capacity * sizeof(archetype)
        );

    storage.graphs =
        (graph*)realloc(storage.graphs, storage.capacity * sizeof(graph));
}
}


