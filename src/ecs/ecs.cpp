#include "gr-base/ecs/ecs.hpp"

#include "gr-base/ecs/entityLocationMap.hpp"
#include "gr-base/ecs/archetypeStorage.hpp"
#include "gr-base/ecs/type.hpp"
#include "gr-base/externals/handle.h"

#include <assert.h>
#include <cstring>

ECS::ECS() : m_emptyArchetypeStorage(nullptr), m_entityLocationMap(std::make_shared<EntityLocationMap>())
{
    auto&&[it, inserted] = m_signArcheTypeMap.emplace(0, std::make_shared<ArchetypeStorage>(0, INVALID_TYPE));

    m_emptyArchetypeStorage = it->second.get();
}

Entity ECS::Create()
{
    Entity e = m_entityLocationMap->create();
    
    EntityID entityID = handle_get_first(e);
    if (entityID >= m_entitesInfo.size())
        m_entitesInfo.resize(entityID + 1, m_emptyArchetypeStorage);

    m_entitesInfo[entityID] = m_emptyArchetypeStorage;

    return e;
}

void ECS::Destroy(Entity e)
{
    if (!m_entityLocationMap->isAlive(e))
        return;

    EntityID entityID = handle_get_first(e);

    ArchetypeStorage* archeType = m_entitesInfo[entityID];
    archeType->Deallocate(e);

    m_entitesInfo[entityID] = m_emptyArchetypeStorage;

    m_entityLocationMap->destroy(e);
}

bool ECS::IsAlive(Entity entity) const
{
    return m_entityLocationMap->isAlive(entity);
}

std::vector<TypeID> ECS::intersection(ArchetypeStorage* src, ArchetypeStorage* dst)
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

void ECS::MoveEntity(ArchetypeStorage* src, ArchetypeStorage* dst, Entity e)
{
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

    m_entitesInfo[entityID] = dst;
}

ArchetypeStorage* ECS::GetOrCreateArcheType(Signature mask, TypeID typeID)
{
    auto it = m_signArcheTypeMap.find(mask);
    if (it != m_signArcheTypeMap.end())
        return it->second.get();

    auto&& [iterator, inserted] =m_signArcheTypeMap.emplace(mask, std::make_shared<ArchetypeStorage>(mask, typeID));

    return iterator->second.get();
}

Signature ECS::GetMask(Entity e) const
{
    if (!m_entityLocationMap->isAlive(e))
        return 0;

    EntityID entityID = handle_get_first(e);

    auto archeType = m_entitesInfo[entityID];

    return archeType->GetMask();
}

void* ECS::AddComponent(Entity e, TypeID typeID, const void* src)
{
    uint32_t entityID = handle_get_first(e);

    ArchetypeStorage* lastArcheType = m_entitesInfo[entityID];

    auto type = Type::GetTypeInfoByID(typeID);

    ArchetypeStorage* dst = lastArcheType->addEdges(typeID);
    if (dst == nullptr)
    {
        Signature newMask = lastArcheType->GetMask() | (1ull << typeID);

        dst = GetOrCreateArcheType(newMask, typeID);

        lastArcheType->LinkAdd(typeID, dst);
    }

    MoveEntity(lastArcheType, dst, e);

    void* ptr = dst->Get(e, type->index);

    ((void(*)(void*, const void*))(type->constructor))(ptr, src);

    return ptr;
}

void* ECS::GetComponent(Entity e, TypeID typeID) const
{
    if (!m_entityLocationMap->isAlive(e))
        return nullptr;

    EntityID entityID = handle_get_first(e);
    if (entityID >= m_entitesInfo.size())
        return nullptr;

    ArchetypeStorage* archeType = m_entitesInfo[entityID];
    if ((archeType->GetMask() & (1ull << typeID)) != (1ull << typeID))
        return nullptr;

    return archeType->Get(e, typeID);
}


void* ECS::GetOrAddComponent(Entity e, TypeID typeID)
{
    void* c = GetComponent(e, typeID);
    if (c != nullptr)
        return c;
    return AddComponent(e, typeID, nullptr);
}

void ECS::DelComponent(Entity e, Hash64 hash)
{
    if (!m_entityLocationMap->isAlive(e))
        return;

    auto type = Type::GetTypeInfoByHash(hash);
    if (type == nullptr)
        return;

    uint32_t entityID = handle_get_first(e);

    ArchetypeStorage* src = m_entitesInfo[entityID];

    Signature oldMask = src->GetMask();
    Signature newMask = oldMask & ~(1ull << type->index);

    ArchetypeStorage* dst = GetOrCreateArcheType(newMask, type->index);
    if (dst == src)
        return;

    MoveEntity(src, dst, e);
}

void ECS::DelComponent(Entity e, const char* name)
{
    DelComponent(e, hash::fnv1_str(name));
}

ArchetypeStorage* ECS::GetArchetype(Signature mask) const
{
    auto it = m_signArcheTypeMap.find(mask);
    if (it == m_signArcheTypeMap.end())
        return nullptr;
    return it->second.get();
}

size_t ECS::GetCount() const
{
    return m_entityLocationMap->GetCount();
}



