#include "gr-base/ecs/entity.hpp"

#include "gr-base/core/mainthreaddispatcher.hpp"
#include "gr-base/core/scene.hpp"
#include "gr-base/ecs/behavior.hpp"
#include "gr-base/ecs/behaviorStorage.hpp"

#include <algorithm>

#include <cstdint>

Entity::Entity(EntityID id) : m_id(id)
{}

Behavior* Entity::addComponent(const char* name)
{
    return addComponent(GetHash(name));
}

Behavior* Entity::getComponent(const char* name) const
{
    return getComponent(GetHash(name));
}

static void OnStart(uint64_t hash, ComponentHandle handle)
{
    auto behavior = BehaviorStorage::GetComponent(hash, handle);
    if (behavior == nullptr)
        return;

    behavior->Start();
}

Behavior* Entity::addComponent(uint64_t hash, ComponentHandle source)
{
    size_t tableID = hash % COMPONENT_TABLE_SIZE;
    if (m_signature.test(tableID))
        return BehaviorStorage::GetComponent(hash, m_lookupTable[tableID]);

    ComponentHandle handle = BehaviorStorage::Allocate(hash, source);
    if (handle == INVALID_COMPONENT_HANDLE)
        return nullptr;

    m_signature.set(tableID, true);

    m_lookupTable[tableID] = handle;

    m_activeHashes.push_back(hash);

    auto behavior = BehaviorStorage::GetComponent(hash, handle);
    behavior->entityID = m_id;
    behavior->Init();

    auto scene = Scene::GetCurrent();
    if (scene != nullptr && scene->bRunning)
        MainThreadDispatcher::Add(Priority::NORMAL, &OnStart, hash, handle);

    return behavior;
}

Behavior* Entity::getComponent(uint64_t hash) const
{
    size_t tableID = hash % COMPONENT_TABLE_SIZE;
    if (m_signature.test(tableID))
        return BehaviorStorage::GetComponent(hash, m_lookupTable[tableID]);
    return nullptr;
}

void Entity::destroy(uint64_t hash)
{
    size_t tableID = hash % COMPONENT_TABLE_SIZE;
    if (!m_signature.test(tableID))
        return;

    m_signature.set(tableID, false);

    ComponentID id = m_lookupTable[tableID];

    m_activeHashes.erase(std::remove(m_activeHashes.begin(), m_activeHashes.end(), hash), m_activeHashes.end());

    BehaviorStorage::Deallocate(hash, id);
}

bool Entity::reinstance(const char* name, Behavior* dest, const Behavior* src)
{
    return BehaviorStorage::Reinstance(GetHash(name), dest, src);
}

std::vector<uint64_t> Entity::GetHashes() const
{
    return m_activeHashes;
}

ComponentHandle Entity::GetID(uint64_t hash) const
{
    size_t tableID = hash % COMPONENT_TABLE_SIZE;
    if (!m_signature.test(tableID))
        return INVALID_COMPONENT_ID;

    return m_lookupTable[tableID];
}

EntityID Entity::getCacheID() const
{
    return m_id;
}


