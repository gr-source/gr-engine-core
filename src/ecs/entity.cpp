#include "gr-base/ecs/entity.hpp"

#include "gr-base/core/gameObject.hpp"

#include "gr-base/ecs/componentregistry.hpp"
#include "gr-base/ecs/componentstorage.hpp"
#include "gr-base/ecs/component.hpp"

#include <algorithm>

Entity::Entity(GameObjectHandle cacheID) : m_cacheID(cacheID), m_bPrefab(false)
{}

Entity::Entity(Entity&& other) noexcept
{
    for (size_t i=0;i<COMPONENT_TABLE_SIZE;i++)
        m_lookupTable[i] = other.m_lookupTable[i];

    m_signature = std::move(other.m_signature);

    m_activeHashes = std::move(other.m_activeHashes);

    m_cacheID = other.m_cacheID;

    m_bPrefab = other.m_bPrefab;
}

Component* Entity::addComponent(const char* name)
{
    return addComponent(fnv1_hash(name));
}

Component* Entity::getComponent(const char* name) const
{
    return getComponent(fnv1_hash(name));
}

// #include <iostream>
Component* Entity::addComponent(uint64_t hash, ComponentHandle source)
{
    size_t tableID = hash & (COMPONENT_TABLE_SIZE - 1);
    if (m_signature.test(tableID))
    {
        auto& lookup = m_lookupTable[tableID];
        return ComponentStorage::GetComponent(lookup.hash, lookup.handle);
    }

    ComponentHandle handle = ComponentStorage::Allocate(hash, source);
    if (handle == HANDLE_INVALID)
        return nullptr;

    m_signature.set(tableID, true);

    m_lookupTable[tableID] = {hash, handle};

    m_activeHashes.push_back(hash);

    TypeInfo* info = ComponentRegistry::Get(hash);
    while (info->parent != (ComponentHash)-1)
    {
        info = ComponentRegistry::Get(info->parent);

        tableID = info->hash & (COMPONENT_TABLE_SIZE - 1);

        m_signature.set(tableID, true);

        m_lookupTable[tableID] = {hash, handle};
    }

    auto behavior = ComponentStorage::GetComponent(hash, handle);
    behavior->cacheID = m_cacheID;

    if (!m_bPrefab)
        behavior->Init();

    return behavior;
}

Component* Entity::getComponent(uint64_t hash) const
{
    size_t tableID = hash & (COMPONENT_TABLE_SIZE - 1);
    if (m_signature.test(tableID))
    {
        auto& lookup = m_lookupTable[tableID];
        return ComponentStorage::GetComponent(lookup.hash, lookup.handle);
    }
    return nullptr;
}

void Entity::destroy(uint64_t hash)
{
    size_t tableID = hash & (COMPONENT_TABLE_SIZE - 1);
    if (!m_signature.test(tableID))
        return;

    ComponentHandle handle = m_lookupTable[tableID].handle;

    auto behavior = ComponentStorage::GetComponent(hash, handle);
    behavior->Destroy();

    m_signature.set(tableID, false);

    m_activeHashes.erase(std::remove(m_activeHashes.begin(), m_activeHashes.end(), hash), m_activeHashes.end());

    ComponentStorage::Deallocate(hash, handle);
}

bool Entity::reinstance(const char* name, Component* dest, const Component* src)
{
    return ComponentStorage::Reinstance(fnv1_hash(name), dest, src);
}

std::vector<uint64_t> Entity::GetHashes() const
{
    return m_activeHashes;
}

ComponentHandle Entity::GetID(uint64_t hash) const
{
    size_t tableID = hash & (COMPONENT_TABLE_SIZE - 1);
    if (!m_signature.test(tableID))
        return INVALID_COMPONENT_ID;

    return m_lookupTable[tableID].handle;
}

#include "gr-base/script/transform.hpp"
void Entity::CallStart()
{
    for (auto& hash : GetHashes())
    {
        auto behavior = ComponentStorage::GetComponent(hash, GetID(hash));
        behavior->Start();
    }

    auto transform = getComponent<Transform>();

    for (auto& childID : transform->getChildren())
    {
        auto gameObject = GameObject::Get(childID);

        gameObject->CallStart();
    }
}

// protected
void Entity::CallDestroy()
{
    for (auto& hash : GetHashes())
    {
        auto behavior = ComponentStorage::GetComponent(hash, GetID(hash));
        behavior->Destroy();
    }

    /*
    auto transform = getComponent<Transform>();

    for (auto& childID : transform->getChildren())
    {
        auto gameObject = GameObject::Get(childID);

        gameObject->CallDestroy();
    }
    */
}


