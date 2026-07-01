#include "gr-base/ecs/world.hpp"

#include "gr-base/core/logger.hpp"

#include "gr-base/ecs/archetypeStorage.hpp"
#include "gr-base/ecs/componentRegistry.hpp"
#include "gr-base/ecs/entityManager.hpp"

#include "gr-base/externals/handle.h"

#include <assert.h>
#include <cstdlib>
#include <cstring>

World::World() : m_emptyArchetypeStorage(INVALID_ARCHETYPE), m_entityLocationMap(std::make_shared<EntityManager>()), m_archetypePool(nullptr), m_count(0), m_capacity(0)
{
    logger::info("[ World ] Create '0x%lX'.", this);

    m_emptyArchetypeStorage = GetOrCreateArcheType(0, INVALID_TYPE);
}

World::~World()
{
    logger::info("[ World ] Destroy '0x%lX'.", this);

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

Entity World::Create()
{
    Entity e = m_entityLocationMap->create();
    
    EntityID entityID = handle_get_first(e);
    if (entityID >= entityRecords.size())
        entityRecords.resize(entityID + 1, { m_emptyArchetypeStorage, engine::ecs::detail::location{ INVALID_CHUNK, INVALID_COMPONENT } });

    entityRecords[entityID] = { m_emptyArchetypeStorage, engine::ecs::detail::location{ INVALID_CHUNK, INVALID_COMPONENT } };

    return e;
}

void World::Destroy(Entity entity)
{
    if (!m_entityLocationMap->isAlive(entity))
        return;

    EntityID entityID = handle_get_first(entity);

    auto& rec = entityRecords[entityID];

    auto& arche = m_archetypePool[rec.archetype];

    Entity moved = INVALID_ENTITY;
    if ((moved = arche.deallocate(rec.location)) != INVALID_ENTITY)
        entityRecords[handle_get_first(moved)].location = rec.location;

    entityRecords[entityID] = { m_emptyArchetypeStorage, engine::ecs::detail::location{ INVALID_CHUNK, INVALID_COMPONENT } };

    m_entityLocationMap->destroy(entity);
}

bool World::IsAlive(Entity entity) const
{
    return m_entityLocationMap->isAlive(entity);
}

ArchetypeID World::GetOrCreateArcheType(Signature mask, TypeID typeID)
{
    auto it = m_signArcheTypeMap.find(mask);
    if (it != m_signArcheTypeMap.end())
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

    ArchetypeLayout layout;
    layout.offsets = (size_t*)malloc(maxTypeID * sizeof(size_t));

    for (auto& type : ComponentRegistry::GetTypesOfMask(mask))
    {
        auto info = ComponentRegistry::GetComponentByID(type);

        layout.stride += info->stride;
    }
    layout.capacity = layout.stride > 0 ? CHUNK_SIZE / layout.stride : 0;

    size_t offset = 0;
    for (auto& type : ComponentRegistry::GetTypesOfMask(mask))
    {
        auto info = ComponentRegistry::GetComponentByID(type);

        layout.offsets[type] = offset;

        offset += info->stride * layout.capacity;
    }


    ArchetypeStorage* arche = m_archetypePool + id;
    new (arche) ArchetypeStorage();

    arche->mask = mask;

    // Define a layout para runtime
    arche->layout = (ArchetypeLayout*)malloc(sizeof(ArchetypeLayout));
    memcpy(arche->layout, &layout, sizeof(ArchetypeLayout));

    if (typeID != INVALID_TYPE)
    {
        if (typeID >= arche->layout->graphy.capacity)
            arche->layout->graphy.grow(typeID + 1);
    }

    // new (&arche) ArchetypeStorage();

    // logger::debug("[World] Create new ArcheType 'id=%d mask=%d ptr=0x%lX'.", id, mask, archeType);

    m_signArcheTypeMap.emplace(mask, id);

    return id;
}

Signature World::GetMask(Entity e) const
{
    if (!m_entityLocationMap->isAlive(e))
        return 0;

    EntityID entityID = handle_get_first(e);
    if (entityID >= entityRecords.size())
        return 0;

    auto& loc = entityRecords[entityID];

    auto& arche = m_archetypePool[loc.archetype];

    return arche.mask;
}

void* World::AddComponent(Entity e, TypeID typeID, const void* component)
{
    if (!m_entityLocationMap->isAlive(e))
        return nullptr;

    auto type = ComponentRegistry::GetComponentByID(typeID);
    if (type == nullptr)
        return nullptr;

    uint32_t entityID = handle_get_first(e);

    ArchetypeID srcID = entityRecords[entityID].archetype;

    ArchetypeStorage* lastArcheType = GetArchetypeByID(srcID);

    ArchetypeID dstID = lastArcheType->addEdges(typeID);
    if (dstID == INVALID_ARCHETYPE)
    {
        Signature newMask = lastArcheType->mask | (1ull << typeID);

        dstID = GetOrCreateArcheType(newMask, typeID);

        LinkAdd(typeID, srcID, dstID);
    }

    MoveEntity(srcID, dstID, e);

    auto dst = GetArchetypeByID(dstID);
    void* ptr = (void*)dst->get(entityRecords[entityID].location, type);

    ((void(*)(void*, const void*))(type->constructor))(ptr, component);

    return ptr;
}

void* World::GetComponent(Entity e, TypeID typeID) const
{
    if (!m_entityLocationMap->isAlive(e))
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

void* World::GetOrAddComponent(Entity e, TypeID typeID)
{
    void* c = GetComponent(e, typeID);
    if (c != nullptr)
        return c;
    return AddComponent(e, typeID, nullptr);
}

bool World::HasComponent(Entity entity, Signature mask) const
{
    if (!m_entityLocationMap->isAlive(entity))
        return false;

    uint32_t entityID = handle_get_first(entity);
    if (entityID >= entityRecords.size())
        return false;

    auto& loc = entityRecords[entityID];
    
    auto arche = GetArchetypeByID(loc.archetype);

    return (arche->mask & mask) == mask;
}

void World::DelComponent(Entity e, Hash64 hash)
{
    if (!m_entityLocationMap->isAlive(e))
        return;

    auto type = ComponentRegistry::GetComponentInfoByHash(hash);
    if (type == nullptr)
        return;

    uint32_t entityID = handle_get_first(e);

    auto srcID = entityRecords[entityID].archetype;
    ArchetypeStorage* src = GetArchetypeByID(srcID);

    Signature oldMask = src->mask;
    Signature newMask = oldMask & ~(1ull << type->index);

    auto dstID = GetOrCreateArcheType(newMask, type->index);
    if (dstID == srcID)
        return;

    MoveEntity(srcID, dstID, e);
}

std::vector<TypeID> World::GetAllComponentType(Entity e) const
{
    return ComponentRegistry::GetTypesOfMask(GetMask(e));
}

size_t World::GetEntityCount() const
{
    return m_entityLocationMap->GetCount();
}

ArchetypeStorage* World::GetArchetypeByMask(Signature mask) const
{
    auto it = m_signArcheTypeMap.find(mask);
    if (it == m_signArcheTypeMap.end())
        return nullptr;
    return m_archetypePool + it->second;
}

void World::MoveEntity(ArchetypeID srcID, ArchetypeID dstID, Entity e)
{
    EntityID entityID = handle_get_first(e);

    ArchetypeStorage* src = GetArchetypeByID(srcID);
    ArchetypeStorage* dst = GetArchetypeByID(dstID);

    engine::ecs::detail::location src_loc = entityRecords[entityID].location;
    engine::ecs::detail::location dst_loc = dst->allocate(e);

    Signature common = src->mask & dst->mask;
    while (common)
    {
        TypeID typeID = __builtin_ctzll(common);

        auto info = ComponentRegistry::GetComponentByID(typeID);

        void* dst_component = dst->get(dst_loc, info);
        void* src_component = src->get(src_loc, info);

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

    entityRecords[entityID] = { dstID, dst_loc };
}

ArchetypeStorage* World::GetArchetypeByID(ArchetypeID id) const
{
    assert(id < m_count);
    return m_archetypePool + id;
}

void World::LinkAdd(TypeID typeID, ArchetypeID srcID, ArchetypeID dstID)
{
    auto src = GetArchetypeByID(srcID);
    auto dst = GetArchetypeByID(dstID);

    if (typeID >= src->layout->graphy.capacity)
        src->layout->graphy.grow(typeID + 1);

    if (typeID >= dst->layout->graphy.capacity)
        dst->layout->graphy.grow(typeID + 1);

    // logger::trace("[ World ] LinkAdd typeID=%d src_edges=%ld dst_rem=%ld", typeID, src->m_addEdges.size(), dst->m_removeEdges.size());

    src->layout->graphy.addEdges[typeID] = dstID;
    dst->layout->graphy.removeEdges[typeID] = srcID;
}

void World::resize()
{
    m_capacity = m_capacity > 0 ? m_capacity * 2 : 2;

    m_archetypePool = (ArchetypeStorage*)realloc(m_archetypePool, m_capacity * sizeof(ArchetypeStorage));
}



