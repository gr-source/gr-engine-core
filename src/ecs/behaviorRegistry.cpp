#include "gr-base/ecs/behaviorRegistry.hpp"

#include "gr-base/ecs/behavior.hpp"

#include <algorithm>

#include <string.h>

// public
void BehaviorRegistry::Unregistry(uint64_t hash)
{
    GetInstance().unregistry(hash);
}

ComponentInfo* BehaviorRegistry::Get(uint64_t hash)
{
    return GetInstance().get(hash);
}

std::vector<uint64_t> BehaviorRegistry::GetActiveBehaviorActive()
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
        ComponentInfo& dest = ((ComponentInfo*)m_infos->vector)[poolID];

        if (replace(&dest, info))
            return;
        return;
    }
    m_bitmask.set(poolID, true);

    m_behaviorActives.push_back(info->hash);

    ComponentInfo& dest = ((ComponentInfo*)m_infos->vector)[poolID];
    memcpy((void *)&dest, (void *)info, sizeof(ComponentInfo));
}

void BehaviorRegistry::unregistry(uint64_t hash)
{
    size_t poolID = hash % COMPONENT_TABLE_SIZE;

    if (!m_bitmask.test(poolID))
        return;
    m_bitmask.set(poolID, false);

    ComponentInfo& info = ((ComponentInfo*)m_infos->vector)[poolID];

    m_behaviorActives.erase(std::remove(m_behaviorActives.begin(), m_behaviorActives.end(), poolID), m_behaviorActives.end());
}

ComponentInfo* BehaviorRegistry::get(uint64_t hash)
{
    size_t poolID = hash % COMPONENT_TABLE_SIZE;
    if (m_bitmask.test(poolID))
        return &((ComponentInfo *)m_infos->vector)[poolID];
    return nullptr;
}

#include <iostream>
bool BehaviorRegistry::replace(ComponentInfo* dest, const ComponentInfo* src)
{
    dest->name = src->name;

    dest->getwrapperbehavior = src->getwrapperbehavior;
    dest->getwrappernext = src->getwrappernext;

    dest->constructor = src->constructor;
    dest->destructor = src->destructor;

    dest->start = src->start;
    dest->update = src->update;
    dest->destroy = src->destroy;

    if (dest->size != src->size)
    {
        std::cout << "Request Resize: " << dest->name << std::endl;
        return true;
    }

    return false;
}


