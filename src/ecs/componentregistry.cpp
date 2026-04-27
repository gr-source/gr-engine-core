#include "gr-base/ecs/componentregistry.hpp"

#include "gr-base/core/logger.hpp"
#include "gr-base/ecs/componentstorage.hpp"

#include "gr-base/externals/array.h"

#include <algorithm>

#include <stdexcept>
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

ComponentRegistry::ComponentRegistry() : m_sparseSet(nullptr), m_pool(nullptr), m_capacity(0), m_count(0)
{
    m_sparseSet = array_create(COMPONENT_TABLE_SIZE, sizeof(ComponentInfoID));
}

ComponentRegistry::~ComponentRegistry()
{
    if (m_sparseSet != nullptr)
        array_free(m_sparseSet);

    if (m_pool != nullptr)
        array_free(m_pool);
}

void ComponentRegistry::registry(ComponentInfo* info)
{
    size_t mapID = info->hash & (COMPONENT_TABLE_SIZE - 1);
    if (m_bitmask.test(mapID))
    {
        ComponentInfoID id = *((ComponentInfoID*)m_sparseSet->vector + mapID);

        ComponentInfo* componentInfo = ((ComponentInfo*)m_pool->vector + id);

        if (reconfigure(componentInfo, info))
        {
            componentInfo->name = info->name;

            ComponentStorage::NotifyUpdate(info->hash, info);
        }
        return;
    }

    if (m_count >= m_capacity)
        reallocate();

    ComponentInfoID id = m_count++;

    *((ComponentInfoID*)m_sparseSet->vector + mapID) = id;

    ComponentInfo& dest = ((ComponentInfo*)m_pool->vector)[id];
    dest.hash = info->hash;
    dest.name = info->name;
    dest.offset = info->offset;
    dest.parent = info->parent;
    dest.stride = info->stride;

    dest.constructor = info->constructor;
    dest.destructor = info->destructor;

    dest.start = info->start;
    dest.update = info->update;
    dest.destroy = info->destroy;

    m_bitmask.set(mapID, true);

    // logger::info("[ComponentRegistry] New component registered '%s\t\t%llu\tparent\t%llu'", dest.name, dest.hash, dest.parent);
    logger::info("[ComponentRegistry] New component registered '%-*s %llu parent %llu'",
             20, dest.name, dest.hash, dest.parent);   
}

void ComponentRegistry::unregistry(ComponentHash hash)
{
    throw std::runtime_error("Falta corrigir aqui");
    /*
    size_t poolID = hash & (COMPONENT_TABLE_SIZE - 1);

    if (!m_bitmask.test(poolID))
        return;
    m_bitmask.set(poolID, false);

    ComponentInfo& info = ((ComponentInfo*)m_infos->vector)[poolID];

    m_behaviorActives.erase(std::remove(m_behaviorActives.begin(), m_behaviorActives.end(), poolID), m_behaviorActives.end());
    */
}

ComponentInfo* ComponentRegistry::get(ComponentHash hash)
{
    size_t mapID = hash & (COMPONENT_TABLE_SIZE - 1);
    if (m_bitmask.test(mapID))
    {
        ComponentInfoID id = *((ComponentInfoID*)m_sparseSet->vector + mapID);

        return (ComponentInfo *)(m_pool->vector) + id;
    }
    return nullptr;
}

bool ComponentRegistry::reconfigure(ComponentInfo* dest, const ComponentInfo* src)
{
    dest->name = src->name;
    dest->parent = src->parent;
    dest->getvetable = src->getvetable;

    if (
            dest->starthash != src->starthash ||
            dest->updatehash != src->updatehash ||
            dest->destroyhash != src->destroyhash
        )
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


