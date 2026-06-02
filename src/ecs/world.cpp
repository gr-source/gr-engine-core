#include "gr-base/ecs/world.hpp"

#include "gr-base/core/logger.hpp"

#include "gr-base/ecs/archetypeStorage.hpp"
#include "gr-base/ecs/entityManager.hpp"
#include "gr-base/ecs/type.hpp"

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

    for (ArchetypeID i=0;i<m_count;i++)
        m_archetypePool[i].~ArchetypeStorage();

    if (m_archetypePool != nullptr)
        free(m_archetypePool);
}

Entity World::Create()
{
    Entity e = m_entityLocationMap->create();
    
    EntityID entityID = handle_get_first(e);
    if (entityID >= entityToLocation.size())
        entityToLocation.resize(entityID + 1, { m_emptyArchetypeStorage, INVALID_COMPONENT });

    entityToLocation[entityID] = { m_emptyArchetypeStorage, INVALID_COMPONENT };

    return e;
}

void World::Destroy(Entity e)
{
    if (!m_entityLocationMap->isAlive(e))
        return;

    EntityID entityID = handle_get_first(e);

    auto& rec = entityToLocation[entityID];

    auto& arche = m_archetypePool[rec.archetype];

    uint32_t lastGlobal = arche.entityCount > 0 ? arche.entityCount - 1 : 0;
    if (arche.Deallocate(rec.slot) && rec.slot != lastGlobal)
    {
        entityToLocation[handle_get_first(arche.slotToEntity[lastGlobal])].slot = rec.slot;
    }

    entityToLocation[entityID] = { m_emptyArchetypeStorage, INVALID_COMPONENT };

    m_entityLocationMap->destroy(e);
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

    ArchetypeID id = m_count++;

    ArchetypeStorage& arche = m_archetypePool[id];
    new (&arche) ArchetypeStorage(mask, typeID);

    // logger::debug("[World] Create new ArcheType 'id=%d mask=%d ptr=0x%lX'.", id, mask, archeType);

    m_signArcheTypeMap.emplace(mask, id);

    return id;
}

Signature World::GetMask(Entity e) const
{
    if (!m_entityLocationMap->isAlive(e))
        return 0;

    EntityID entityID = handle_get_first(e);
    if (entityID >= entityToLocation.size())
        return 0;

    auto& arche = m_archetypePool[entityToLocation[entityID].archetype];

    return arche.m_mask;
}

void* World::AddComponent(Entity e, TypeID typeID, const void* component)
{
    if (!m_entityLocationMap->isAlive(e))
        return nullptr;

    auto type = Type::GetTypeInfoByID(typeID);
    if (type == nullptr)
        return nullptr;

    uint32_t entityID = handle_get_first(e);

    ArchetypeID srcID = entityToLocation[entityID].archetype;

    ArchetypeStorage* lastArcheType = GetArchetypeByID(srcID);

    ArchetypeID dstID = lastArcheType->addEdges(typeID);
    if (dstID == INVALID_ARCHETYPE)
    {
        Signature newMask = lastArcheType->m_mask | (1ull << typeID);

        dstID = GetOrCreateArcheType(newMask, typeID);

        LinkAdd(typeID, srcID, dstID);
    }

    MoveEntity(srcID, dstID, e);

    auto dst = GetArchetypeByID(dstID);
    void* ptr = dst->Get(entityToLocation[entityID].slot, type);

    ((void(*)(void*, const void*))(type->constructor))(ptr, component);

    return ptr;
}

void* World::GetComponent(Entity e, TypeID typeID) const
{
    if (!m_entityLocationMap->isAlive(e))
        return nullptr;

    auto type = Type::GetTypeInfoByID(typeID);

    EntityID entityID = handle_get_first(e);
    if (entityID >= entityToLocation.size())
        return nullptr;

    auto& rec = entityToLocation[entityID];

    ArchetypeStorage* archeType = GetArchetypeByID(rec.archetype);
    if ((archeType->m_mask & (1ull << typeID)) != (1ull << typeID))
        return nullptr;

    return archeType->Get(rec.slot, type);
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
    if (entityID >= entityToLocation.size())
        return false;

    auto& loc = entityToLocation[entityID];
    
    auto arche = GetArchetypeByID(loc.archetype);

    return (arche->m_mask & mask) == mask;
}

void World::DelComponent(Entity e, Hash64 hash)
{
    if (!m_entityLocationMap->isAlive(e))
        return;

    auto type = Type::GetTypeInfoByHash(hash);
    if (type == nullptr)
        return;

    uint32_t entityID = handle_get_first(e);

    auto srcID = entityToLocation[entityID].archetype;
    ArchetypeStorage* src = GetArchetypeByID(srcID);

    Signature oldMask = src->m_mask;
    Signature newMask = oldMask & ~(1ull << type->index);

    auto dstID = GetOrCreateArcheType(newMask, type->index);
    if (dstID == srcID)
        return;

    MoveEntity(srcID, dstID, e);
}

std::vector<TypeID> World::GetAllComponentType(Entity e) const
{
    Signature mask = GetMask(e);

    return GetArchetypeByMask(mask)->GetTypes();
}

size_t World::GetCount(ArchetypeID id) const
{
    return GetArchetypeByID(id)->entityCount;
}

size_t World::GetCount() const
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

    ComponentID srcSlotIndex = entityToLocation[entityID].slot;
    ComponentID dstSlotIndex = dst->Allocate(e);

    Signature common = src->m_mask & dst->m_mask;
    while (common)
    {
        TypeID typeID = __builtin_ctzll(common);

        auto type = Type::GetTypeInfoByID(typeID);

        void* dst_component = dst->GetRaw(typeID) + dstSlotIndex * type->stride;
        void* src_component = src->GetRaw(typeID) + srcSlotIndex * type->stride;

        if (type->globalCpy)
        {
            memcpy(
                dst_component,
                src_component,
                type->stride
            );

        } else
        {
            ((void(*)(void*, const void*))(type->move))(dst_component, src_component);
        }

        common &= (common - 1);
    }

    if (srcSlotIndex != INVALID_COMPONENT)
    {
        uint32_t lastGlobal = src->entityCount > 0 ? src->entityCount - 1 : 0;
        if (src->Deallocate(srcSlotIndex) && srcSlotIndex != lastGlobal)
        {
            entityToLocation[handle_get_first(src->slotToEntity[lastGlobal])].slot = srcSlotIndex;
        }
        // src->Deallocate(srcIndex);
    }

    entityToLocation[entityID] = { dstID, dstSlotIndex };
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

    if (typeID >= src->m_addEdges.size())
        src->m_addEdges.resize(typeID + 1, INVALID_ARCHETYPE);

    if (typeID >= dst->m_removeEdges.size())
        dst->m_removeEdges.resize(typeID + 1, INVALID_ARCHETYPE);

    // logger::trace("[ World ] LinkAdd typeID=%d src_edges=%ld dst_rem=%ld", typeID, src->m_addEdges.size(), dst->m_removeEdges.size());

    src->m_addEdges[typeID] = dstID;
    dst->m_removeEdges[typeID] = srcID;
}

void World::resize()
{
    ArchetypeID oldCapacity = m_capacity;
    m_capacity = m_capacity > 0 ? m_capacity * 2 : 2;

    // logger::debug("[ World ] [ RESIZE ] 'm_archetypePool' of=0x%lX.", this);

    // logger::trace("[ World ] oldSize='%ld' newSize='%ld' oldptr='0x%lX'.", oldCapacity, m_capacity, m_archetypePool);

    ArchetypeStorage* newPool = (ArchetypeStorage*)malloc(m_capacity * sizeof(ArchetypeStorage));

    for (ArchetypeID i=0;i<m_count;i++)
    {
        ArchetypeStorage* oldArcheType = m_archetypePool + i;
        ArchetypeStorage* newArcheType = newPool + i;

        new (newArcheType) ArchetypeStorage(std::move(*oldArcheType));
        oldArcheType->~ArchetypeStorage();
    }

    free(m_archetypePool);
    m_archetypePool = newPool;

    // logger::trace("[ World ] newptr='0x%lX'.", m_archetypePool);
}



