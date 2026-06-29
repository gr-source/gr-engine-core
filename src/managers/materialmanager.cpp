#include "gr-base/managers/materialmanager.hpp"

#include "gr-base/core/geometry/material.hpp"

#include "gr-base/externals/handle.h"

#include <cstdlib>

MaterialManager::MaterialManager() : m_count(0), m_capacity(0), m_slots(nullptr), m_freeID(0)
{}

MaterialHandle MaterialManager::create(AssetHash hash)
{
    if (m_count >= m_capacity)
        resize();

    uint32_t id = m_freeID;

    Slot& slot = m_slots[id];
    slot.hash = hash;

    m_freeID = slot.next;

    m_count++;

    MaterialHandle h = handle_create(id, slot.version);

    Material* material = &m_materialPool[id];
    new (material) Material();

    if (hash != INVALID_HASH64)
        m_materialMap.emplace(hash, h);

    return h;
}

MaterialHandle MaterialManager::find(AssetHash hash) const
{
    auto it = m_materialMap.find(hash);
    if (it == m_materialMap.end())
        return HANDLE_INVALID;
    return it->second;
}

Material *MaterialManager::get(MaterialHandle h)
{
    uint32_t index = handle_get_first(h);
    uint32_t version = handle_get_second(h);

    Slot& slot = m_slots[index];
    if (slot.version != version)
        return nullptr;

    return &m_materialPool[index];
}

void MaterialManager::destroy(MaterialHandle h)
{
    uint32_t index = handle_get_first(h);
    uint32_t version = handle_get_second(h);

    Slot& slot = m_slots[index];
    if (slot.version != version)
        return;

    Material& material = m_materialPool[index];
    material.~Material();

    if (slot.hash != INVALID_HASH64)
        m_materialMap.erase(slot.hash);

    m_freeID = index;

    slot.next = m_freeID;
    slot.version++;

    m_count--;
}

size_t MaterialManager::GetCount() const
{
    return m_count;
}

// private
void MaterialManager::resize()
{
    size_t oldCapacity = m_capacity;

    m_capacity = m_capacity > 0 ? m_capacity * 2 : 2;

    m_materialPool.resize(m_capacity);

    // m_materialPool = (Material*)realloc(m_materialPool, m_capacity * sizeof(Material));
    m_slots = (Slot*)realloc(m_slots, m_capacity * sizeof(Slot));

    addFreeID(oldCapacity);
}

void MaterialManager::addFreeID(size_t oldCapacity)
{
    size_t lastElement = m_capacity - 1;
    for (size_t i=oldCapacity;i<lastElement;i++)
        m_slots[i] = { static_cast<uint32_t>(i) + 1, 1 };

    m_slots[lastElement] = { m_freeID, 1 };
    m_freeID = static_cast<uint32_t>(oldCapacity);
}



