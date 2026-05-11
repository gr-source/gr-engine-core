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

ComponentInfo* ComponentRegistry::Get(ComponentHash hash)
{
    return GetInstance().get(hash);
}

ComponentInfo* ComponentRegistry::GetAllComponentInfo()
{
    return GetInstance().getAllComponentInfo();
}

ComponentInfoID ComponentRegistry::GetCount()
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
    // m_sparseSet = array_create(COMPONENT_TABLE_SIZE, sizeof(ComponentInfoID));

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

bool ComponentRegistry::registry(ComponentInfo* info)
{
    size_t infoTableID = info->hash & (COMPONENT_TABLE_SIZE - 1);
    if (m_lookUpMap.find(infoTableID) != m_lookUpMap.end())
    {
        ComponentInfoID& id = m_lookUpMap[infoTableID];

        ComponentInfo* componentInfo = ((ComponentInfo*)m_pool->vector + id);

        if (reconfigure(componentInfo, info))
        {
            componentInfo->name = info->name;

            ComponentStorage::NotifyUpdate(info->hash, info);
        }
        return false;
    }

    if (m_count >= m_capacity)
        reallocate();

    ComponentInfoID id = m_count++;

    ComponentInfo& dest = ((ComponentInfo*)m_pool->vector)[id];
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

    ComponentInfo& info = ((ComponentInfo*)m_pool->vector)[it->second];

    logger::trace("[ComponentRegistry] Unregistry: '%-*s 0x%lX parent 0x%lX'",
             20, info.name, info.hash, info.parent);

    ComponentInfoID lastID = --m_count;
    if (lastID != it->second)
    {
        ComponentInfo& lastInfo = ((ComponentInfo*)m_pool->vector)[lastID];

        size_t targetInfoTableID = lastInfo.hash & (COMPONENT_TABLE_SIZE - 1);

        m_lookUpMap[targetInfoTableID] = it->second;

        memcpy(&info, &lastInfo, sizeof(ComponentInfo));
        memset(&lastInfo, 0, sizeof(ComponentInfo));
    }

    m_lookUpMap.erase(infoTableID);
}

ComponentInfo* ComponentRegistry::get(ComponentHash hash)
{
    size_t infoTableID = hash & (COMPONENT_TABLE_SIZE - 1);

    auto it = m_lookUpMap.find(infoTableID);
    if (it == m_lookUpMap.end())
        return nullptr;

    return ((ComponentInfo *)(m_pool->vector)) + it->second;
}

bool ComponentRegistry::reconfigure(ComponentInfo* dest, const ComponentInfo* src)
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

        m_pool = array_resize(m_pool, m_capacity, sizeof(ComponentInfo));
    } else
    {
        m_capacity = 1;

        m_pool = array_create(m_capacity, sizeof(ComponentInfo));
    }
}


