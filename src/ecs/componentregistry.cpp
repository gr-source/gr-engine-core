#include "gr-base/ecs/componentregistry.hpp"

#include "gr-base/core/logger.hpp"
#include "gr-base/ecs/componentstorage.hpp"

#include "gr-base/externals/array.h"

#include <string.h>

// public
void ComponentRegistry::Unregistry(ComponentHash hash)
{
    GetInstance().unregistry(hash);
}

TypeInfo* ComponentRegistry::Get(ComponentHash hash)
{
    return GetInstance().get(hash);
}

TypeInfo* ComponentRegistry::GetAllTypeInfo()
{
    return GetInstance().getAllTypeInfo();
}

TypeInfoID ComponentRegistry::GetCount()
{
    return GetInstance().getCount();
}

// private
ComponentRegistry& ComponentRegistry::GetInstance()
{
    static ComponentRegistry s_ComponentRegistry;
    return s_ComponentRegistry;
}

ComponentRegistry::ComponentRegistry() : /* m_sparseSet(nullptr),*/ m_pool(nullptr), m_capacity(0), m_count(0)
{
    // m_sparseSet = array_create(COMPONENT_TABLE_SIZE, sizeof(TypeInfoID));

    logger::info("[ComponentRegistry] Initialize.");
}

ComponentRegistry::~ComponentRegistry()
{
    /*
    if (m_sparseSet != nullptr)
        array_free(m_sparseSet);
    */

    if (m_pool != nullptr)
        array_free(m_pool);

    logger::info("[ComponentRegistry] Shutdown.");
}

bool ComponentRegistry::registry(TypeInfo* info)
{
    size_t infoTableID = info->hash & (COMPONENT_TABLE_SIZE - 1);
    if (m_lookUpMap.find(infoTableID) != m_lookUpMap.end())
    {
        TypeInfoID& id = m_lookUpMap[infoTableID];

        TypeInfo* componentInfo = ((TypeInfo*)m_pool->vector + id);

        if (reconfigure(componentInfo, info))
        {
            componentInfo->name = info->name;

            ComponentStorage::NotifyUpdate(info->hash, info);
        }
        return false;
    }

    if (m_count >= m_capacity)
        reallocate();

    TypeInfoID id = m_count++;

    TypeInfo& dest = ((TypeInfo*)m_pool->vector)[id];
    dest.vtable = info->vtable;
    dest.hash = info->hash;
    dest.name = info->name;
    dest.offset = info->offset;
    dest.parent = info->parent;
    dest.stride = info->stride;

    dest.constructor = info->constructor;
    dest.destructor = info->destructor;

    m_lookUpMap.emplace(infoTableID, id);

    logger::trace("[ComponentRegistry] Registry: '%-*s 0x%lX parent 0x%lX'",
             20, dest.name, dest.hash, dest.parent);

    return true;
}

void ComponentRegistry::unregistry(ComponentHash hash)
{
    size_t infoTableID = hash & (COMPONENT_TABLE_SIZE - 1);
    auto it = m_lookUpMap.find(infoTableID);
    if (it == m_lookUpMap.end())
        return;

    TypeInfo& info = ((TypeInfo*)m_pool->vector)[it->second];

    logger::trace("[ComponentRegistry] Unregistry: '%-*s 0x%lX parent 0x%lX'",
             20, info.name, info.hash, info.parent);

    TypeInfoID lastID = --m_count;
    if (lastID != it->second)
    {
        TypeInfo& lastInfo = ((TypeInfo*)m_pool->vector)[lastID];

        size_t targetInfoTableID = lastInfo.hash & (COMPONENT_TABLE_SIZE - 1);

        m_lookUpMap[targetInfoTableID] = it->second;

        memcpy(&info, &lastInfo, sizeof(TypeInfo));
        memset(&lastInfo, 0, sizeof(TypeInfo));
    }

    m_lookUpMap.erase(infoTableID);
}

TypeInfo* ComponentRegistry::get(ComponentHash hash)
{
    size_t infoTableID = hash & (COMPONENT_TABLE_SIZE - 1);

    auto it = m_lookUpMap.find(infoTableID);
    if (it == m_lookUpMap.end())
        return nullptr;

    return ((TypeInfo *)(m_pool->vector)) + it->second;
}

bool ComponentRegistry::reconfigure(TypeInfo* dest, const TypeInfo* src)
{
    dest->name = src->name;
    dest->parent = src->parent;

    if (dest->vtable != src->vtable)
        return true;

    if (dest->stride != src->stride)
        return true;

    return false;
}

void ComponentRegistry::reallocate()
{
    if (m_capacity)
    {
        m_capacity *= 2;

        m_pool = array_resize(m_pool, m_capacity, sizeof(TypeInfo));
    } else
    {
        m_capacity = 1;

        m_pool = array_create(m_capacity, sizeof(TypeInfo));
    }
}


