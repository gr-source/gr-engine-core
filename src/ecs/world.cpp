#include "gr-base/ecs/world.hpp"

#include "gr-base/core/logger.hpp"

#include "gr-base/ecs/componentRegistry.hpp"
#include "gr-base/ecs/detail.hpp"
#include "gr-base/ecs/entityManager.hpp"

#include "gr-base/externals/handle.h"
#include "gr-base/script/camera.hpp"

#include <assert.h>
#include <cstdlib>
#include <cstring>
#include <memory>

#define CHUNK_SIZE (1024 * 16)

namespace engine::ecs
{

world::world()
{
    m_emptyArchetypeStorage = GetOrCreateArcheType(0, INVALID_TYPE);
}

world::~world()
{
    for (size_t i=0;i<m_count;i++)
    {
        auto& arche = m_archetypePool[i];

        auto layout = arche.layout;
        if (layout != nullptr)
        {
            if (layout->graphy.addEdges != nullptr)
                free(layout->graphy.addEdges);

            if (layout->graphy.removeEdges != nullptr)
                free(layout->graphy.removeEdges);

            if (layout->offsets != nullptr)
                free(layout->offsets);

            free(layout);
        }

        if (arche.freeList != nullptr)
            free(arche.freeList);

        /// TODO: Vai causa vazamentos de memoia em non-trivally
        if (arche.chunks != nullptr)
            free(arche.chunks);
    }

    if (m_archetypePool != nullptr)
        free(m_archetypePool);
}

Entity world::Create()
{
    Entity e = entityManager.create();
    EntityID entityID = handle_get_first(e);
    if (entityID >= entityRecords.size())
        entityRecords.resize(entityID + 1, {
                .mask = INVALID_SIGNATURE,
                .archetype = m_emptyArchetypeStorage,

                .location = {
                    .chunk = INVALID_CHUNK,
                    .row = 0
                }
            });

    entityRecords[entityID] = {
        .mask = INVALID_SIGNATURE,
        .archetype = m_emptyArchetypeStorage,

        .location = {
            .chunk = INVALID_CHUNK,
            .row = 0
        }
    };

    return e;
}

void world::Destroy(Entity entity)
{
    if (!entityManager.isAlive(entity))
        return;

    EntityID entityID = handle_get_first(entity);

    auto& rec = entityRecords[entityID];

    auto arche = GetArchetypeByID(rec.archetype);

    Entity moved = INVALID_ENTITY;
    if ((moved = arche->deallocate(rec.location)) != INVALID_ENTITY)
        entityRecords[handle_get_first(moved)].location = rec.location;

    entityRecords[entityID] = {
        .mask = INVALID_SIGNATURE,
        .archetype = m_emptyArchetypeStorage,

        .location = {
            .chunk = INVALID_CHUNK,
            .row = 0
        }
    };

    entityManager.destroy(entity);
}

bool world::IsAlive(Entity entity) const
{
    return entityManager.isAlive(entity);
}

ArchetypeID world::GetOrCreateArcheType(Signature mask, TypeID typeID)
{
    auto it = signatureToArchetype.find(mask);
    if (it != signatureToArchetype.end())
        return it->second;

    if (m_count >= m_capacity)
        resize();

    TypeID maxTypeID = 0;

    Signature temp = mask;
    while (temp)
    {
        TypeID id = __builtin_ctzll(temp);
        if (id >= maxTypeID)
            maxTypeID = id + 1;
        
        temp &= (temp - 1);
    }

    ArchetypeID id = static_cast<ArchetypeID>(m_count++);

    engine::ecs::detail::layout layout;
    if (maxTypeID > 0)
    {
        layout.numOffsets = maxTypeID;
        layout.offsets = (size_t*)malloc(maxTypeID * sizeof(size_t));
    }

    size_t stride = 0;
    size_t maxAlign = 1;

    maxAlign = std::max(maxAlign, alignof(Chunk));
    maxAlign = std::max(maxAlign, alignof(Entity));

    for (auto type : ComponentRegistry::GetTypesOfMask(mask))
    {
        auto info = ComponentRegistry::GetComponentByID(type);

        stride += info->stride;
        maxAlign = std::max(maxAlign, info->alignment);
    }

    layout.stride = stride;

    layout.capacity = (stride > 0)
        ? (CHUNK_SIZE / stride)
        : 0;

    size_t localOffset = 0;
    for (auto type : ComponentRegistry::GetTypesOfMask(mask))
    {
        auto info = ComponentRegistry::GetComponentByID(type);

        localOffset = align_up(localOffset, info->alignment);

        layout.offsets[type] = localOffset;

        localOffset += info->stride * layout.capacity;
    }

    size_t headerAndEntitiesSize =
        sizeof(Chunk) +
        (sizeof(Entity) * layout.capacity);

    size_t baseOffset =
        align_up(headerAndEntitiesSize, maxAlign);

    size_t data_size =
        stride > 0 ?
            stride * layout.capacity : 0;

    layout.payloadSize =
        align_up(
                baseOffset + data_size,
                maxAlign
            );

    // layout.payloadSize =
    //     stride > 0 ?
    //         align_up(
    //                 buffer_size,
    //                 maxAlign
    //             ) : 0;

    layout.alignment = maxAlign;


    archetype* arche = GetArchetypeByID(id);
    new (arche) archetype();

    arche->mask = mask;

    // Define a layout para runtime
    arche->layout = (engine::ecs::detail::layout*)malloc(sizeof(engine::ecs::detail::layout));
    memcpy(arche->layout, &layout, sizeof(engine::ecs::detail::layout));

    // if (typeID != INVALID_TYPE)
    {
        if (maxTypeID >= arche->layout->graphy.capacity)
            arche->layout->graphy.grow(maxTypeID + 1);
    }

    signatureToArchetype.emplace(mask, id);

    return id;
}

Signature world::GetMask(Entity e) const
{
    if (!entityManager.isAlive(e))
        return 0;

    EntityID entityID = handle_get_first(e);
    if (entityID >= entityRecords.size())
        return 0;

    return entityRecords[entityID].mask;
}

engine::ecs::detail::records world::Allocate(Entity entity, TypeID type)
{
    Signature mask = (1ull << type);

    uint32_t entityID = handle_get_first(entity);

    ArchetypeID srcID = entityRecords[entityID].archetype;

    auto lastArcheType = GetArchetypeByID(srcID);
    if ((lastArcheType->mask & mask) == mask)
        std::cout << "Has be" << std::endl;
        // return srcID;

    ArchetypeID dstID = lastArcheType->addEdges(type);
    if (dstID == INVALID_ARCHETYPE)
    {
        Signature newMask = lastArcheType->mask | mask;

        dstID = GetOrCreateArcheType(newMask, type);

        LinkAdd(type, srcID, dstID);
    }

    MoveEntity(srcID, dstID, entity);

    // auto dst = GetArchetypeByID(dstID);

    // void* ptr = (void*)dst->get(entityRecords[entityID].location, type);

    // type->constructor(ptr, component);

    return entityRecords[entityID];
}

void* world::AddComponentByID(Entity entity, TypeID type)
{
    assert(entityManager.isAlive(entity));

    auto info = ComponentRegistry::GetComponentByID(type);
    assert(info != nullptr);

    auto record = Allocate(entity, type);

    auto arche = GetArchetypeByID(record.archetype);

    uint8_t* raw = (uint8_t*)arche->getRaw(record.location.chunk, type);

    void* component = (void*)(raw + record.location.row * info->stride);
    info->constructor(component, nullptr);

    return component;
}

void* world::GetComponentByID(Entity entity, TypeID type) const
{
    assert(entityManager.isAlive(entity));
    if (!entityManager.isAlive(entity))
        return nullptr;

    auto info = ComponentRegistry::GetComponentByID(type);
    assert(info != nullptr);
    if (info == nullptr)
        return nullptr;

    uint32_t entityID = handle_get_first(entity);
    assert(entityID < entityRecords.size());
    if (entityID > entityRecords.size())
        return nullptr;

    auto& record = entityRecords[entityID];
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

    auto arche = GetArchetypeByID(record.archetype);

    uint8_t* raw = (uint8_t*)arche->getRaw(record.location.chunk, type);

    return (void*)(raw + record.location.row * info->stride);
}

/*
void* world::GetComponent(Entity e, TypeID typeID) const
{
    if (!entityManager.isAlive(e))
        return nullptr;

    auto type = ComponentRegistry::GetComponentByID(typeID);
    if (type == nullptr)
        return nullptr;

    EntityID entityID = handle_get_first(e);
    if (entityID >= entityRecords.size())
        return nullptr;

    auto& rec = entityRecords[entityID];

    auto arche = GetArchetypeByID(rec.archetype);
    if ((arche->mask & (1ull << typeID)) != (1ull << typeID))
        return nullptr;

    return (void*)arche->get(rec.location, type);
}

void* world::GetOrAddComponent(Entity e, TypeID typeID)
{
    void* c = GetComponent(e, typeID);
    if (c != nullptr)
        return c;
    return AddComponent(e, typeID, nullptr);
}
*/

/*
bool world::HasComponent(Entity entity, Signature mask) const
{
    if (!entityManager.isAlive(entity))
        return false;

    uint32_t entityID = handle_get_first(entity);
    if (entityID >= entityRecords.size())
        return false;

    auto& loc = entityRecords[entityID];
    
    auto arche = GetArchetypeByID(loc.archetype);

    return (arche->mask & mask) == mask;
}
*/

/*
void world::DelComponent(Entity e, Hash64 hash)
{
    if (!entityManager.isAlive(e))
        return;

    auto type = ComponentRegistry::GetComponentInfoByHash(hash);
    if (type == nullptr)
        return;

    uint32_t entityID = handle_get_first(e);

    auto srcID = entityRecords[entityID].archetype;
    engine::ecs::dynamic::archetype* src = GetArchetypeByID(srcID);

    Signature oldMask = src->mask;
    Signature newMask = oldMask & ~(1ull << type->index);

    auto dstID = GetOrCreateArcheType(newMask, type->index);
    if (dstID == srcID)
        return;

    MoveEntity(srcID, dstID, e);
}
*/

std::vector<TypeID> world::GetAllComponentType(Entity e) const
{
    return ComponentRegistry::GetTypesOfMask(GetMask(e));
}

size_t world::GetEntityCount() const
{
    return entityManager.count;
}

void world::MoveEntity(ArchetypeID srcID, ArchetypeID dstID, Entity e)
{
    EntityID entityID = handle_get_first(e);

    archetype* src = GetArchetypeByID(srcID);
    archetype* dst = GetArchetypeByID(dstID);

    engine::ecs::detail::location src_loc = entityRecords[entityID].location;
    engine::ecs::detail::location dst_loc = dst->allocate(e);

    Signature common = src->mask & dst->mask;
    while (common)
    {
        TypeID typeID = __builtin_ctzll(common);

        auto info = ComponentRegistry::GetComponentByID(typeID);

        uint8_t* dst_raw = (uint8_t*)dst->getRaw(dst_loc.chunk, typeID);
        uint8_t* src_raw = (uint8_t*)src->getRaw(src_loc.chunk, typeID);

        void* dst_component = (void*)(dst_raw + dst_loc.row * info->stride);
        void* src_component = (void*)(src_raw + src_loc.row * info->stride);

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

        common &= (common - 1);
    }

    Entity moved = INVALID_ENTITY;
    if (src_loc.chunk != INVALID_CHUNK && (moved = src->deallocate(src_loc)) != INVALID_ENTITY)
    {
        entityRecords[handle_get_first(moved)].location = src_loc;
    }

    entityRecords[entityID] = {
        .mask = dst->mask,
        .archetype = dstID,
        .location = dst_loc
    };
}

archetype* world::GetArchetypeByID(ArchetypeID id) const
{
    if (id >= m_count)
        return nullptr;
    return &m_archetypePool[id];
}

void world::LinkAdd(TypeID typeID, ArchetypeID srcID, ArchetypeID dstID)
{
    auto src = GetArchetypeByID(srcID);
    auto dst = GetArchetypeByID(dstID);

    if (typeID >= src->layout->graphy.capacity)
        src->layout->graphy.grow(typeID + 1);

    if (typeID >= dst->layout->graphy.capacity)
        dst->layout->graphy.grow(typeID + 1);

    // logger::trace("[ world ] LinkAdd typeID=%d src_edges=%ld dst_rem=%ld", typeID, src->m_addEdges.size(), dst->m_removeEdges.size());

    src->layout->graphy.addEdges[typeID] = dstID;
    dst->layout->graphy.removeEdges[typeID] = srcID;
}

void world::resize()
{
    m_capacity = m_capacity > 0 ? m_capacity * 2 : 2;

    m_archetypePool =
        (archetype*)realloc(
            m_archetypePool,
            m_capacity * sizeof(archetype)
        );
}
}


