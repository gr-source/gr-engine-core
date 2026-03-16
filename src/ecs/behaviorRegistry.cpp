#include "gr-base/ecs/behaviorRegistry.hpp"

#include "gr-base/ecs/behaviorStorage.hpp"

#include <algorithm>

#include <string.h>

// public
void BehaviorRegistry::Unregistry(ComponentHash hash)
{
    GetInstance().unregistry(hash);
}

ComponentInfo* BehaviorRegistry::Get(ComponentHash hash)
{
    return GetInstance().get(hash);
}

std::vector<ComponentHash> BehaviorRegistry::GetActiveBehaviorActive()
{
    return GetInstance().m_behaviorActives;
}

array* BehaviorRegistry::GetBehaviorInfos()
{
    return GetInstance().m_infos;
}

// private
BehaviorRegistry& BehaviorRegistry::GetInstance()
{
    static BehaviorRegistry s_BehaviorRegistry;
    return s_BehaviorRegistry;
}

BehaviorRegistry::BehaviorRegistry() : m_infos(nullptr)
{
    m_infos = array_create(COMPONENT_TABLE_SIZE, sizeof(ComponentInfo));
}

BehaviorRegistry::~BehaviorRegistry()
{
    if (m_infos != nullptr)
        array_free(m_infos);
}

void BehaviorRegistry::registry(ComponentInfo* info)
{
    size_t poolID = info->hash % COMPONENT_TABLE_SIZE;
    if (m_bitmask.test(poolID))
    {
        ComponentInfo *dest = ((ComponentInfo*)m_infos->vector) + poolID;

        if (reconfigure(dest, info))
        {
            dest->name = info->name;

            BehaviorStorage::NotifyUpdate(info->hash, info);
        }
        return;
    }
    m_bitmask.set(poolID, true);

    m_behaviorActives.push_back(info->hash);

    ComponentInfo& dest = ((ComponentInfo*)m_infos->vector)[poolID];
    memcpy((void *)&dest, (void *)info, sizeof(ComponentInfo));
}

void BehaviorRegistry::unregistry(ComponentHash hash)
{
    size_t poolID = hash % COMPONENT_TABLE_SIZE;

    if (!m_bitmask.test(poolID))
        return;
    m_bitmask.set(poolID, false);

    ComponentInfo& info = ((ComponentInfo*)m_infos->vector)[poolID];

    m_behaviorActives.erase(std::remove(m_behaviorActives.begin(), m_behaviorActives.end(), poolID), m_behaviorActives.end());
}

ComponentInfo* BehaviorRegistry::get(ComponentHash hash)
{
    size_t poolID = hash % COMPONENT_TABLE_SIZE;
    if (m_bitmask.test(poolID))
        return (ComponentInfo *)(m_infos->vector) + poolID;
    return nullptr;
}

bool BehaviorRegistry::reconfigure(ComponentInfo* dest, const ComponentInfo* src)
{
    dest->name = src->name;
    dest->parent = src->parent;

    // dest->getwrapperbehavior = src->getwrapperbehavior;
    // dest->getwrappernext = src->getwrappernext;
    //
    // dest->constructor = src->constructor;
    // dest->destructor = src->destructor;

    if (
            dest->starthash != src->starthash ||
            dest->updatehash != src->updatehash ||
            dest->destroyhash != src->destroyhash
        )
    {
        return true;
        // std::cout << "Request: " << src->name << " reinstance" << std::endl;
    }

    // dest->start = src->start;
    // dest->update = src->update;
    // dest->destroy = src->destroy;

    if (dest->stride != src->stride)
    {
        // std::cout << "Request: " << dest->name << " chunk resize" << std::endl;
        // BehaviorStorage::RequestResize(dest->hash, src->size);

        // dest->size = src->size;
        return true;
    }

    return false;
}


