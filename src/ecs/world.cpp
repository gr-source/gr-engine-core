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

    for (ArcheTypeID i=0;i<m_count;i++)
        m_archetypePool[i].~ArchetypeStorage();

    if (m_archetypePool != nullptr)
        free(m_archetypePool);
}

Entity World::Create()
{
    Entity e = m_entityLocationMap->create();
    
    EntityID entityID = handle_get_first(e);
    if (entityID >= m_entitesInfo.size())
        m_entitesInfo.resize(entityID + 1, m_emptyArchetypeStorage);

    m_entitesInfo[entityID] = m_emptyArchetypeStorage;

    return e;
}

void World::Destroy(Entity e)
{
    if (!m_entityLocationMap->isAlive(e))
        return;

    EntityID entityID = handle_get_first(e);

    ArchetypeStorage* archeType = m_archetypePool + m_entitesInfo[entityID];
    archeType->Deallocate(e);

    m_entitesInfo[entityID] = m_emptyArchetypeStorage;

    m_entityLocationMap->destroy(e);
}

bool World::IsAlive(Entity entity) const
{
    return m_entityLocationMap->isAlive(entity);
}

std::vector<TypeID> World::intersection(ArchetypeStorage* src, ArchetypeStorage* dst)
{
    std::vector<TypeID> typeIDs;

    Signature common = src->GetMask() & dst->GetMask();

    while (common)
    {
        TypeID typeID = __builtin_ctzll(common);
        typeIDs.push_back(typeID);

        common &= (common - 1);
    }

    return typeIDs;
}

void World::MoveEntity(ArcheTypeID srcID, ArcheTypeID dstID, Entity e)
{
    ArchetypeStorage* src = GetArchetypeByID(srcID);
    ArchetypeStorage* dst = GetArchetypeByID(dstID);

    ComponentID srcIndex = src->GetIndexOf(e);
    ComponentID dstIndex = dst->Allocate(e);

    assert(srcIndex != INVALID_COMPONENT || dstIndex != INVALID_COMPONENT);

    for (TypeID t : intersection(src, dst))
    {
        auto type = Type::GetTypeInfoByID(t);

        void* dst_component = dst->GetRaw(t) + dstIndex * type->stride;
        void* src_component = src->GetRaw(t) + srcIndex * type->stride;

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
    }

    src->Deallocate(e);

    EntityID entityID = handle_get_first(e);

    m_entitesInfo[entityID] = dstID;
}

ArcheTypeID World::GetOrCreateArcheType(Signature mask, TypeID typeID)
{
    auto it = m_signArcheTypeMap.find(mask);
    if (it != m_signArcheTypeMap.end())
        return it->second;

    if (m_count >= m_capacity)
        resize();

    ArcheTypeID id = m_count++;

    auto archeType = m_archetypePool + id;
    new (archeType) ArchetypeStorage(mask, typeID);

    // logger::debug("[World] Create new ArcheType 'id=%d mask=%d ptr=0x%lX'.", id, mask, archeType);

    m_signArcheTypeMap.emplace(mask, id);

    return id;
}

Signature World::GetMask(Entity e) const
{
    if (!m_entityLocationMap->isAlive(e))
        return 0;

    EntityID entityID = handle_get_first(e);

    auto archeType = GetArchetypeByID(m_entitesInfo[entityID]);

    return archeType->GetMask();
}

void* World::AddComponent(Entity e, TypeID typeID, const void* component)
{
    if (!m_entityLocationMap->isAlive(e))
        return nullptr;

    uint32_t entityID = handle_get_first(e);

    ArcheTypeID srcID = m_entitesInfo[entityID];

    ArchetypeStorage* lastArcheType = GetArchetypeByID(srcID);

    auto type = Type::GetTypeInfoByID(typeID);

    ArcheTypeID dstID = lastArcheType->addEdges(typeID);
    if (dstID == INVALID_ARCHETYPE)
    {
        Signature newMask = lastArcheType->GetMask() | (1ull << typeID);

        dstID = GetOrCreateArcheType(newMask, typeID);

        LinkAdd(typeID, srcID, dstID);
    }

    MoveEntity(srcID, dstID, e);

    auto dst = GetArchetypeByID(dstID);
    void* ptr = dst->Get(e, type->index);

    ((void(*)(void*, const void*))(type->constructor))(ptr, component);

    return ptr;
}

void* World::GetComponent(Entity e, TypeID typeID) const
{
    if (!m_entityLocationMap->isAlive(e))
        return nullptr;

    EntityID entityID = handle_get_first(e);
    if (entityID >= m_entitesInfo.size())
        return nullptr;

    ArchetypeStorage* archeType = GetArchetypeByID(m_entitesInfo[entityID]);
    if ((archeType->GetMask() & (1ull << typeID)) != (1ull << typeID))
        return nullptr;

    return archeType->Get(e, typeID);
}


void* World::GetOrAddComponent(Entity e, TypeID typeID)
{
    void* c = GetComponent(e, typeID);
    if (c != nullptr)
        return c;
    return AddComponent(e, typeID, nullptr);
}

void World::DelComponent(Entity e, Hash64 hash)
{
    if (!m_entityLocationMap->isAlive(e))
        return;

    auto type = Type::GetTypeInfoByHash(hash);
    if (type == nullptr)
        return;

    uint32_t entityID = handle_get_first(e);

    auto srcID = m_entitesInfo[entityID];
    ArchetypeStorage* src = GetArchetypeByID(srcID);

    Signature oldMask = src->GetMask();
    Signature newMask = oldMask & ~(1ull << type->index);

    auto dstID = GetOrCreateArcheType(newMask, type->index);
    if (dstID == srcID)
        return;

    MoveEntity(srcID, dstID, e);
}

void World::DelComponent(Entity e, const char* name)
{
    DelComponent(e, hash::fnv1_str(name));
}

std::vector<TypeID> World::GetAllComponentType(Entity e) const
{
    Signature mask = GetMask(e);

    return GetArchetypeByMask(mask)->GetTypes();
}

size_t World::GetCount(ArcheTypeID id) const
{
    return GetArchetypeByID(id)->Count();
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

ArchetypeStorage* World::GetArchetypeByID(ArcheTypeID id) const
{
    assert(id >= m_count);
    return m_archetypePool + id;
}

void World::LinkAdd(TypeID typeID, ArcheTypeID srcID, ArcheTypeID dstID)
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
    ArcheTypeID oldCapacity = m_capacity;
    m_capacity = m_capacity > 0 ? m_capacity * 2 : 2;

    // logger::debug("[ World ] [ RESIZE ] 'm_archetypePool' of=0x%lX.", this);

    // logger::trace("[ World ] oldSize='%ld' newSize='%ld' oldptr='0x%lX'.", oldCapacity, m_capacity, m_archetypePool);

    ArchetypeStorage* newPool = (ArchetypeStorage*)malloc(m_capacity * sizeof(ArchetypeStorage));

    for (ArcheTypeID i=0;i<m_count;i++)
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



